#pragma once

#include <QObject>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace GadAI {

/**
 * @brief File management service for attachments and media
 */
class FileVault : public QObject
{
    Q_OBJECT

public:
    explicit FileVault(const QString &vaultPath, QObject *parent = nullptr);

    /**
     * @brief Initialize the file vault directory structure
     */
    bool initialize();

    /**
     * @brief Store a file in the vault
     * @param sourcePath Source file path
     * @param category Category for organization (images, documents, audio, etc.)
     * @return New file path in vault, empty string on failure
     */
    QString storeFile(const QString &sourcePath, const QString &category = QString());

    /**
     * @brief Store file data directly in the vault
     * @param data File data
     * @param fileName Original file name
     * @param category Category for organization
     * @return New file path in vault, empty string on failure
     */
    QString storeFileData(const QByteArray &data, const QString &fileName, const QString &category = QString());

    /**
     * @brief Remove a file from the vault
     * @param vaultPath Path within the vault
     * @return true if successful
     */
    bool removeFile(const QString &vaultPath);

    /**
     * @brief Check if a file exists in the vault
     * @param vaultPath Path within the vault
     * @return true if file exists
     */
    bool fileExists(const QString &vaultPath) const;

    /**
     * @brief Get file information
     * @param vaultPath Path within the vault
     * @return QFileInfo for the file
     */
    QFileInfo getFileInfo(const QString &vaultPath) const;

    /**
     * @brief Read file data from vault
     * @param vaultPath Path within the vault
     * @return File data, empty on failure
     */
    QByteArray readFile(const QString &vaultPath) const;

    /**
     * @brief Get the full system path for a vault file
     * @param vaultPath Path within the vault
     * @return Full system path
     */
    QString getFullPath(const QString &vaultPath) const;

    /**
     * @brief Get relative vault path from full system path
     * @param fullPath Full system path
     * @return Relative vault path, empty if not in vault
     */
    QString getVaultPath(const QString &fullPath) const;

    /**
     * @brief Clean up orphaned files not referenced by any attachments
     * @param referencedPaths List of currently referenced vault paths
     * @return Number of files cleaned up
     */
    int cleanupOrphanedFiles(const QStringList &referencedPaths);

    /**
     * @brief Get vault usage statistics
     */
    struct VaultStats {
        qint64 totalSize;
        int totalFiles;
        int imageFiles;
        int documentFiles;
        int audioFiles;
        int otherFiles;
    };
    VaultStats getStats() const;

    /**
     * @brief Get supported file extensions for each category
     */
    static QStringList getSupportedExtensions(const QString &category);
    static QString getCategoryForFile(const QString &fileName);

signals:
    void fileStored(const QString &vaultPath);
    void fileRemoved(const QString &vaultPath);

private:
    QString generateUniqueFileName(const QString &originalName, const QString &category) const;
    QString sanitizeFileName(const QString &fileName) const;
    bool createDirectoryStructure();
    void scanDirectory(const QString &dirPath, VaultStats &stats) const;

    QString m_vaultPath;
    QDir m_vaultDir;
    
    static const QStringList IMAGE_EXTENSIONS;
    static const QStringList DOCUMENT_EXTENSIONS;
    static const QStringList AUDIO_EXTENSIONS;
};

} // namespace GadAI
