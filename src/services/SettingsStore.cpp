#include "SettingsStore.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincred.h>
#elif defined(Q_OS_MACOS)
#include <Security/Security.h>
#elif defined(Q_OS_LINUX)
// For Linux, we'll use a simple encrypted file storage
// In production, you might want to use libsecret
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#endif

namespace GadAI {

SettingsStore::SettingsStore(const QString &configDir, QObject *parent)
    : QObject(parent)
    , m_configDir(configDir)
{
    QString settingsPath = m_configDir + "/settings.ini";
    m_settings = new QSettings(settingsPath, QSettings::IniFormat, this);
    
    qDebug() << "SettingsStore initialized with config dir:" << m_configDir;
    qDebug() << "Settings file:" << settingsPath;
}

SettingsStore::~SettingsStore()
{
    sync();
}

QVariant SettingsStore::value(const QString &key, const QVariant &defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(key, defaultValue);
}

void SettingsStore::setValue(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&m_mutex);
    QVariant oldValue = m_settings->value(key);
    
    m_settings->setValue(key, value);
    
    if (oldValue != value) {
        emit settingChanged(key, value);
    }
}

void SettingsStore::remove(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    m_settings->remove(key);
}

bool SettingsStore::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->contains(key);
}

QStringList SettingsStore::allKeys() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->allKeys();
}

void SettingsStore::clear()
{
    QMutexLocker locker(&m_mutex);
    m_settings->clear();
}

void SettingsStore::sync()
{
    QMutexLocker locker(&m_mutex);
    m_settings->sync();
}

QString SettingsStore::getKeychainService() const
{
    return "GadAI";
}

QString SettingsStore::getKeychainKey(const QString &key) const
{
    return QString("gadai_%1").arg(key);
}

#ifdef Q_OS_WIN
bool SettingsStore::storeSecret(const QString &key, const QString &secret)
{
    QString fullKey = getKeychainKey(key);
    QString service = getKeychainService();
    
    CREDENTIALA cred = {};  // Initialize all fields to zero/nullptr
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = const_cast<char*>(fullKey.toLocal8Bit().constData());
    cred.Comment = nullptr;
    cred.LastWritten = {};
    cred.CredentialBlobSize = secret.length();
    cred.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<char*>(secret.toLocal8Bit().constData()));
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    cred.AttributeCount = 0;
    cred.Attributes = nullptr;
    cred.TargetAlias = nullptr;
    cred.UserName = const_cast<char*>(service.toLocal8Bit().constData());
    
    if (CredWriteA(&cred, 0)) {
        qDebug() << "Stored secret for key:" << key;
        return true;
    } else {
        qWarning() << "Failed to store secret for key:" << key << "Error:" << GetLastError();
        return false;
    }
}

QString SettingsStore::getSecret(const QString &key) const
{
    QString fullKey = getKeychainKey(key);
    
    PCREDENTIALA pcred;
    if (CredReadA(fullKey.toLocal8Bit().constData(), CRED_TYPE_GENERIC, 0, &pcred)) {
        QString secret = QString::fromLocal8Bit(reinterpret_cast<char*>(pcred->CredentialBlob), pcred->CredentialBlobSize);
        CredFree(pcred);
        return secret;
    } else {
        DWORD error = GetLastError();
        if (error != ERROR_NOT_FOUND) {
            qWarning() << "Failed to get secret for key:" << key << "Error:" << error;
        }
        return QString();
    }
}

bool SettingsStore::removeSecret(const QString &key)
{
    QString fullKey = getKeychainKey(key);
    
    if (CredDeleteA(fullKey.toLocal8Bit().constData(), CRED_TYPE_GENERIC, 0)) {
        qDebug() << "Removed secret for key:" << key;
        return true;
    } else {
        DWORD error = GetLastError();
        if (error != ERROR_NOT_FOUND) {
            qWarning() << "Failed to remove secret for key:" << key << "Error:" << error;
        }
        return error == ERROR_NOT_FOUND; // Consider it success if it didn't exist
    }
}

bool SettingsStore::hasSecret(const QString &key) const
{
    QString fullKey = getKeychainKey(key);
    
    PCREDENTIALA pcred;
    if (CredReadA(fullKey.toLocal8Bit().constData(), CRED_TYPE_GENERIC, 0, &pcred)) {
        CredFree(pcred);
        return true;
    }
    
    return false;
}

#elif defined(Q_OS_MACOS)

bool SettingsStore::storeSecret(const QString &key, const QString &secret)
{
    QString fullKey = getKeychainKey(key);
    QString service = getKeychainService();
    
    QByteArray serviceData = service.toUtf8();
    QByteArray keyData = fullKey.toUtf8();
    QByteArray secretData = secret.toUtf8();
    
    // First try to update existing item
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,
        serviceData.length(), serviceData.constData(),
        keyData.length(), keyData.constData(),
        NULL, NULL, NULL
    );
    
    if (status == errSecSuccess) {
        // Update existing
        SecKeychainItemRef itemRef;
        status = SecKeychainFindGenericPassword(
            NULL,
            serviceData.length(), serviceData.constData(),
            keyData.length(), keyData.constData(),
            NULL, NULL, &itemRef
        );
        
        if (status == errSecSuccess) {
            status = SecKeychainItemModifyAttributesAndData(
                itemRef, NULL,
                secretData.length(), secretData.constData()
            );
            CFRelease(itemRef);
        }
    } else {
        // Create new
        status = SecKeychainAddGenericPassword(
            NULL,
            serviceData.length(), serviceData.constData(),
            keyData.length(), keyData.constData(),
            secretData.length(), secretData.constData(),
            NULL
        );
    }
    
    if (status == errSecSuccess) {
        qDebug() << "Stored secret for key:" << key;
        return true;
    } else {
        qWarning() << "Failed to store secret for key:" << key << "Status:" << status;
        return false;
    }
}

QString SettingsStore::getSecret(const QString &key) const
{
    QString fullKey = getKeychainKey(key);
    QString service = getKeychainService();
    
    QByteArray serviceData = service.toUtf8();
    QByteArray keyData = fullKey.toUtf8();
    
    UInt32 passwordLength;
    void *passwordData;
    
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,
        serviceData.length(), serviceData.constData(),
        keyData.length(), keyData.constData(),
        &passwordLength, &passwordData, NULL
    );
    
    if (status == errSecSuccess) {
        QString secret = QString::fromUtf8(static_cast<char*>(passwordData), passwordLength);
        SecKeychainItemFreeContent(NULL, passwordData);
        return secret;
    } else {
        if (status != errSecItemNotFound) {
            qWarning() << "Failed to get secret for key:" << key << "Status:" << status;
        }
        return QString();
    }
}

bool SettingsStore::removeSecret(const QString &key)
{
    QString fullKey = getKeychainKey(key);
    QString service = getKeychainService();
    
    QByteArray serviceData = service.toUtf8();
    QByteArray keyData = fullKey.toUtf8();
    
    SecKeychainItemRef itemRef;
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,
        serviceData.length(), serviceData.constData(),
        keyData.length(), keyData.constData(),
        NULL, NULL, &itemRef
    );
    
    if (status == errSecSuccess) {
        status = SecKeychainItemDelete(itemRef);
        CFRelease(itemRef);
        
        if (status == errSecSuccess) {
            qDebug() << "Removed secret for key:" << key;
            return true;
        } else {
            qWarning() << "Failed to remove secret for key:" << key << "Status:" << status;
            return false;
        }
    }
    
    return true; // Consider it success if it didn't exist
}

bool SettingsStore::hasSecret(const QString &key) const
{
    QString fullKey = getKeychainKey(key);
    QString service = getKeychainService();
    
    QByteArray serviceData = service.toUtf8();
    QByteArray keyData = fullKey.toUtf8();
    
    OSStatus status = SecKeychainFindGenericPassword(
        NULL,
        serviceData.length(), serviceData.constData(),
        keyData.length(), keyData.constData(),
        NULL, NULL, NULL
    );
    
    return status == errSecSuccess;
}

#else // Linux and other platforms

bool SettingsStore::storeSecret(const QString &key, const QString &secret)
{
    // Simple encrypted file storage for Linux
    // In production, consider using libsecret
    
    QString secretsDir = m_configDir + "/secrets";
    QDir().mkpath(secretsDir);
    
    QString filePath = secretsDir + "/" + getKeychainKey(key) + ".enc";
    
    // Simple XOR encryption with a fixed key (not secure for production)
    QByteArray secretData = secret.toUtf8();
    QByteArray key_bytes = "GadAI_Secret_Key_2025"; // In production, use proper encryption
    
    for (int i = 0; i < secretData.size(); ++i) {
        secretData[i] = secretData[i] ^ key_bytes[i % key_bytes.size()];
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(secretData);
        file.close();
        qDebug() << "Stored secret for key:" << key;
        return true;
    } else {
        qWarning() << "Failed to store secret for key:" << key;
        return false;
    }
}

QString SettingsStore::getSecret(const QString &key) const
{
    QString secretsDir = m_configDir + "/secrets";
    QString filePath = secretsDir + "/" + getKeychainKey(key) + ".enc";
    
    QFile file(filePath);
    if (!file.exists()) {
        return QString();
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray encryptedData = file.readAll();
        file.close();
        
        // Decrypt with same XOR key
        QByteArray key_bytes = "GadAI_Secret_Key_2025";
        
        for (int i = 0; i < encryptedData.size(); ++i) {
            encryptedData[i] = encryptedData[i] ^ key_bytes[i % key_bytes.size()];
        }
        
        return QString::fromUtf8(encryptedData);
    }
    
    return QString();
}

bool SettingsStore::removeSecret(const QString &key)
{
    QString secretsDir = m_configDir + "/secrets";
    QString filePath = secretsDir + "/" + getKeychainKey(key) + ".enc";
    
    QFile file(filePath);
    if (file.exists()) {
        bool success = file.remove();
        if (success) {
            qDebug() << "Removed secret for key:" << key;
        } else {
            qWarning() << "Failed to remove secret for key:" << key;
        }
        return success;
    }
    
    return true; // Consider success if file didn't exist
}

bool SettingsStore::hasSecret(const QString &key) const
{
    QString secretsDir = m_configDir + "/secrets";
    QString filePath = secretsDir + "/" + getKeychainKey(key) + ".enc";
    
    return QFile::exists(filePath);
}

#endif

} // namespace GadAI
