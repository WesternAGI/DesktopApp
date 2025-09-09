#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSettings>
#include <QMutex>

namespace DesktopApp {

/**
 * @brief Secure settings store with keychain integration
 */
class SettingsStore : public QObject
{
    Q_OBJECT

public:
    explicit SettingsStore(const QString &configDir, QObject *parent = nullptr);
    ~SettingsStore();

    /**
     * @brief Get a setting value
     */
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief Set a setting value
     */
    void setValue(const QString &key, const QVariant &value);

    // Temporary backward compatibility wrappers (deprecated) -----------------
    // Older UI code still calls get()/set(); keep wrappers until all sites migrated.
    QVariant get(const QString &key, const QVariant &defaultValue = QVariant()) const { return value(key, defaultValue); }
    void set(const QString &key, const QVariant &value) { setValue(key, value); }

    /**
     * @brief Remove a setting
     */
    void remove(const QString &key);

    /**
     * @brief Check if a setting exists
     */
    bool contains(const QString &key) const;

    /**
     * @brief Get all keys
     */
    QStringList allKeys() const;

    /**
     * @brief Clear all settings
     */
    void clear();

    /**
     * @brief Sync settings to disk
     */
    void sync();

    // Secure keychain operations for API keys
    /**
     * @brief Store a secret in the system keychain
     * @param key The key identifier
     * @param secret The secret value
     * @return true if successful
     */
    bool storeSecret(const QString &key, const QString &secret);

    /**
     * @brief Retrieve a secret from the system keychain
     * @param key The key identifier
     * @return The secret value, or empty string if not found
     */
    QString getSecret(const QString &key) const;

    /**
     * @brief Remove a secret from the system keychain
     * @param key The key identifier
     * @return true if successful
     */
    bool removeSecret(const QString &key);

    /**
     * @brief Check if a secret exists in the keychain
     * @param key The key identifier
     * @return true if the secret exists
     */
    bool hasSecret(const QString &key) const;

signals:
    void settingChanged(const QString &key, const QVariant &value);

private:
    QString getKeychainService() const;
    QString getKeychainKey(const QString &key) const;
    
    mutable QMutex m_mutex;
    QSettings *m_settings;
    QString m_configDir;
};

} // namespace DesktopApp
