#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>
#include <QVector>

namespace GadAI {

/**
 * @brief User preferences and settings
 */
struct UserPreference {
    QString id;
    QString theme;
    double fontScale;
    bool telemetryOptIn;
    QStringList keychainIds;

    UserPreference();
    QJsonObject toJson() const;
    static UserPreference fromJson(const QJsonObject &json);
};

/**
 * @brief Chat conversation entity
 */
struct Conversation {
    QString id;
    QString title;
    QDateTime createdAt;
    QDateTime updatedAt;
    bool pinned;
    bool archived;
    bool deleted; // soft delete (trash)
    int sortOrder; // manual ordering for pinned
    QString providerId;
    QString modelName;
    QJsonObject metadata;

    Conversation();
    Conversation(const QString &title);
    
    QJsonObject toJson() const;
    static Conversation fromJson(const QJsonObject &json);
    
    bool isValid() const;
    void updateTimestamp();
};

/**
 * @brief Message roles
 */
enum class MessageRole {
    User,
    Assistant,
    System
};

QString messageRoleToString(MessageRole role);
MessageRole messageRoleFromString(const QString &roleStr);

/**
 * @brief Individual message within a conversation
 */
struct Message {
    QString id;
    QString conversationId;
    MessageRole role;
    QString text;
    QDateTime createdAt;
    QJsonObject metadata;
    QString parentId; // For message threading/editing
    bool isStreaming;

    Message();
    Message(const QString &convId, MessageRole role, const QString &text);
    
    QJsonObject toJson() const;
    static Message fromJson(const QJsonObject &json);
    
    bool isValid() const;
    QString roleString() const;
};

/**
 * @brief File attachment types
 */
enum class AttachmentType {
    Image,
    PDF,
    Text,
    Audio,
    Other
};

QString attachmentTypeToString(AttachmentType type);
AttachmentType attachmentTypeFromString(const QString &typeStr);

/**
 * @brief File attachment entity
 */
struct Attachment {
    QString id;
    QString messageId;
    AttachmentType type;
    QString fileName;
    QString filePath;
    QString mimeType;
    qint64 fileSize;
    QDateTime createdAt;
    QJsonObject metadata;

    Attachment();
    Attachment(const QString &msgId, const QString &fileName, const QString &filePath);
    
    QJsonObject toJson() const;
    static Attachment fromJson(const QJsonObject &json);
    
    bool isValid() const;
    QString typeString() const;
    bool exists() const;
};

/**
 * @brief AI provider account configuration
 */
struct ProviderAccount {
    QString id;
    QString provider;
    QString label;
    QString endpoint;
    QString apiKeyRef; // Reference to keychain entry
    QString defaultModel;
    QJsonObject parameters;
    bool enabled;
    QDateTime createdAt;

    ProviderAccount();
    ProviderAccount(const QString &provider, const QString &label);
    
    QJsonObject toJson() const;
    static ProviderAccount fromJson(const QJsonObject &json);
    
    bool isValid() const;
};

/**
 * @brief Saved prompt template
 */
struct Prompt {
    QString id;
    QString name;
    QString text;
    QJsonObject variables;
    QString category;
    QDateTime createdAt;
    QDateTime updatedAt;

    Prompt();
    Prompt(const QString &name, const QString &text);
    
    QJsonObject toJson() const;
    static Prompt fromJson(const QJsonObject &json);
    
    bool isValid() const;
    QString processTemplate(const QJsonObject &values) const;
};

/**
 * @brief Search result for messages
 */
struct SearchResult {
    QString messageId;
    QString conversationId;
    QString snippet;
    double relevance;
    QDateTime timestamp;

    SearchResult();
    SearchResult(const QString &msgId, const QString &convId, 
                const QString &snippet, double relevance);
    
    bool isValid() const;
};

// Type aliases for collections
using ConversationList = QVector<Conversation>;
using MessageList = QVector<Message>;
using AttachmentList = QVector<Attachment>;
using ProviderAccountList = QVector<ProviderAccount>;
using PromptList = QVector<Prompt>;
using SearchResultList = QVector<SearchResult>;

} // namespace GadAI
