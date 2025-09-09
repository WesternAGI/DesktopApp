#include "FileVault.h"
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QUuid>
#include <QDebug>
#include <QDirIterator>

namespace DesktopApp {

// Static file extension definitions
const QStringList FileVault::IMAGE_EXTENSIONS = {
    "jpg", "jpeg", "png", "gif", "bmp", "tiff", "webp", "svg"
};

const QStringList FileVault::DOCUMENT_EXTENSIONS = {
    "pdf", "doc", "docx", "txt", "md", "rtf", "odt", "pages"
};

const QStringList FileVault::AUDIO_EXTENSIONS = {
    "mp3", "wav", "m4a", "aac", "ogg", "flac", "wma"
};

FileVault::FileVault(const QString &vaultPath, QObject *parent)
    : QObject(parent)
    , m_vaultPath(vaultPath)
    , m_vaultDir(vaultPath)
{
}

bool FileVault::initialize()
{
    qDebug() << "Initializing FileVault at:" << m_vaultPath;
    
    if (!createDirectoryStructure()) {
        qCritical() << "Failed to create vault directory structure";
        return false;
    }
    
    qDebug() << "FileVault initialized successfully";
    return true;
}

QString FileVault::storeFile(const QString &sourcePath, const QString &category)
{
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists()) {
        qWarning() << "Source file does not exist:" << sourcePath;
        return QString();
    }
    
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot read source file:" << sourcePath;
        return QString();
    }
    
    QByteArray data = sourceFile.readAll();
    sourceFile.close();
    
    QFileInfo sourceInfo(sourcePath);
    QString fileName = sourceInfo.fileName();
    
    return storeFileData(data, fileName, category);
}

QString FileVault::storeFileData(const QByteArray &data, const QString &fileName, const QString &category)
{
    if (data.isEmpty() || fileName.isEmpty()) {
        qWarning() << "Invalid file data or name";
        return QString();
    }
    
    // Determine category if not specified
    QString fileCategory = category.isEmpty() ? getCategoryForFile(fileName) : category;
    
    // Generate unique file name
    QString uniqueFileName = generateUniqueFileName(fileName, fileCategory);
    
    // Create category directory if needed
    QString categoryDir = m_vaultPath + "/" + fileCategory;
    QDir().mkpath(categoryDir);
    
    // Full path for the new file
    QString vaultFilePath = categoryDir + "/" + uniqueFileName;
    QString relativeVaultPath = fileCategory + "/" + uniqueFileName;
    
    // Write file
    QFile vaultFile(vaultFilePath);
    if (!vaultFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write to vault file:" << vaultFilePath;
        return QString();
    }
    
    qint64 written = vaultFile.write(data);
    vaultFile.close();
    
    if (written != data.size()) {
        qWarning() << "Failed to write complete file data";
        removeFile(relativeVaultPath);
        return QString();
    }
    
    qDebug() << "Stored file in vault:" << relativeVaultPath << "(" << data.size() << "bytes)";
    emit fileStored(relativeVaultPath);
    
    return relativeVaultPath;
}

bool FileVault::removeFile(const QString &vaultPath)
{
    QString fullPath = getFullPath(vaultPath);
    QFile file(fullPath);
    
    if (!file.exists()) {
        qWarning() << "File does not exist in vault:" << vaultPath;
        return false;
    }
    
    bool success = file.remove();
    if (success) {
        qDebug() << "Removed file from vault:" << vaultPath;
        emit fileRemoved(vaultPath);
    } else {
        qWarning() << "Failed to remove file from vault:" << vaultPath;
    }
    
    return success;
}

bool FileVault::fileExists(const QString &vaultPath) const
{
    QString fullPath = getFullPath(vaultPath);
    return QFile::exists(fullPath);
}

QFileInfo FileVault::getFileInfo(const QString &vaultPath) const
{
    QString fullPath = getFullPath(vaultPath);
    return QFileInfo(fullPath);
}

QByteArray FileVault::readFile(const QString &vaultPath) const
{
    QString fullPath = getFullPath(vaultPath);
    QFile file(fullPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot read vault file:" << vaultPath;
        return QByteArray();
    }
    
    return file.readAll();
}

QString FileVault::getFullPath(const QString &vaultPath) const
{
    return m_vaultPath + "/" + vaultPath;
}

QString FileVault::getVaultPath(const QString &fullPath) const
{
    if (!fullPath.startsWith(m_vaultPath)) {
        return QString();
    }
    
    QString relativePath = fullPath.mid(m_vaultPath.length());
    if (relativePath.startsWith("/")) {
        relativePath = relativePath.mid(1);
    }
    
    return relativePath;
}

int FileVault::cleanupOrphanedFiles(const QStringList &referencedPaths)
{
    QSet<QString> referenced = QSet<QString>(referencedPaths.begin(), referencedPaths.end());
    QStringList allFiles;
    int removedCount = 0;
    
    // Scan all files in vault
    QDirIterator it(m_vaultPath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString fullPath = it.next();
        QFileInfo info(fullPath);
        
        if (info.isFile()) {
            QString vaultPath = getVaultPath(fullPath);
            if (!vaultPath.isEmpty() && !referenced.contains(vaultPath)) {
                if (removeFile(vaultPath)) {
                    removedCount++;
                }
            }
        }
    }
    
    qDebug() << "Cleaned up" << removedCount << "orphaned files from vault";
    return removedCount;
}

FileVault::VaultStats FileVault::getStats() const
{
    VaultStats stats = {};
    
    if (!m_vaultDir.exists()) {
        return stats;
    }
    
    scanDirectory(m_vaultPath, stats);
    return stats;
}

QStringList FileVault::getSupportedExtensions(const QString &category)
{
    QString lowerCategory = category.toLower();
    
    if (lowerCategory == "images" || lowerCategory == "image") {
        return IMAGE_EXTENSIONS;
    } else if (lowerCategory == "documents" || lowerCategory == "document") {
        return DOCUMENT_EXTENSIONS;
    } else if (lowerCategory == "audio") {
        return AUDIO_EXTENSIONS;
    }
    
    // Return all supported extensions
    QStringList allExtensions;
    allExtensions << IMAGE_EXTENSIONS << DOCUMENT_EXTENSIONS << AUDIO_EXTENSIONS;
    return allExtensions;
}

QString FileVault::getCategoryForFile(const QString &fileName)
{
    QFileInfo info(fileName);
    QString extension = info.suffix().toLower();
    
    if (IMAGE_EXTENSIONS.contains(extension)) {
        return "images";
    } else if (DOCUMENT_EXTENSIONS.contains(extension)) {
        return "documents";
    } else if (AUDIO_EXTENSIONS.contains(extension)) {
        return "audio";
    } else {
        return "other";
    }
}

QString FileVault::generateUniqueFileName(const QString &originalName, const QString &category) const
{
    QFileInfo info(originalName);
    QString baseName = sanitizeFileName(info.baseName());
    QString extension = info.suffix().toLower();
    
    // Add timestamp and UUID for uniqueness
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    
    QString uniqueName = QString("%1_%2_%3.%4")
                         .arg(baseName)
                         .arg(timestamp)
                         .arg(uuid)
                         .arg(extension);
    
    // Ensure the file doesn't exist (very unlikely but better safe)
    QString categoryDir = m_vaultPath + "/" + category;
    int counter = 1;
    while (QFile::exists(categoryDir + "/" + uniqueName)) {
        uniqueName = QString("%1_%2_%3_%4.%5")
                     .arg(baseName)
                     .arg(timestamp)
                     .arg(uuid)
                     .arg(counter++)
                     .arg(extension);
    }
    
    return uniqueName;
}

QString FileVault::sanitizeFileName(const QString &fileName) const
{
    QString sanitized = fileName;
    
    // Remove or replace invalid characters
    const QString invalidChars = "<>:\"/\\|?*";
    for (QChar c : invalidChars) {
        sanitized.replace(c, '_');
    }
    
    // Remove control characters
    sanitized.remove(QRegularExpression("[\\x00-\\x1F\\x7F]"));
    
    // Limit length
    if (sanitized.length() > 100) {
        sanitized = sanitized.left(100);
    }
    
    // Ensure not empty
    if (sanitized.isEmpty()) {
        sanitized = "file";
    }
    
    return sanitized;
}

bool FileVault::createDirectoryStructure()
{
    // Create main vault directory
    if (!m_vaultDir.exists() && !m_vaultDir.mkpath(m_vaultPath)) {
        qCritical() << "Failed to create vault directory:" << m_vaultPath;
        return false;
    }
    
    // Create category subdirectories
    QStringList categories = {"images", "documents", "audio", "other"};
    for (const QString &category : categories) {
        QString categoryPath = m_vaultPath + "/" + category;
        QDir categoryDir(categoryPath);
        if (!categoryDir.exists() && !categoryDir.mkpath(categoryPath)) {
            qWarning() << "Failed to create category directory:" << categoryPath;
        }
    }
    
    qDebug() << "Vault directory structure created successfully";
    return true;
}

void FileVault::scanDirectory(const QString &dirPath, VaultStats &stats) const
{
    QDirIterator it(dirPath, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo info(filePath);
        
        if (info.isFile()) {
            stats.totalFiles++;
            stats.totalSize += info.size();
            
            QString extension = info.suffix().toLower();
            if (IMAGE_EXTENSIONS.contains(extension)) {
                stats.imageFiles++;
            } else if (DOCUMENT_EXTENSIONS.contains(extension)) {
                stats.documentFiles++;
            } else if (AUDIO_EXTENSIONS.contains(extension)) {
                stats.audioFiles++;
            } else {
                stats.otherFiles++;
            }
        }
    }
}

} // namespace DesktopApp
