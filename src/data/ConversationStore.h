#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "Models.h"

namespace GadAI {

/**
 * @brief Database store for conversations, messages, and related data
 */
class ConversationStore : public QObject
{
    Q_OBJECT

public:
    explicit ConversationStore(QSqlDatabase &database, QObject *parent = nullptr);
    
    /**
     * @brief Initialize database tables and run migrations
     */
    bool initialize();
    
    // Conversation operations
    bool createConversation(const Conversation &conversation);
    bool updateConversation(const Conversation &conversation);
    bool deleteConversation(const QString &conversationId);
    Conversation getConversation(const QString &conversationId) const;
    ConversationList getAllConversations() const;
    ConversationList getRecentConversations(int limit = 50) const;
    ConversationList getPinnedConversations(int limit = 200) const;
    ConversationList getArchivedConversations(int limit = 200) const;
    ConversationList getTrashConversations(int limit = 200) const; // deleted
    ConversationList getPinnedConversations() const;
    ConversationList searchConversations(const QString &query) const;
    
    // Message operations
    bool createMessage(const Message &message);
    bool updateMessage(const Message &message);
    bool deleteMessage(const QString &messageId);
    Message getMessage(const QString &messageId) const;
    MessageList getMessagesForConversation(const QString &conversationId) const;
    MessageList getRecentMessages(const QString &conversationId, int limit = 100) const;
    
    // Attachment operations
    bool createAttachment(const Attachment &attachment);
    bool deleteAttachment(const QString &attachmentId);
    Attachment getAttachment(const QString &attachmentId) const;
    AttachmentList getAttachmentsForMessage(const QString &messageId) const;
    
    // Provider account operations
    bool createProviderAccount(const ProviderAccount &account);
    bool updateProviderAccount(const ProviderAccount &account);
    bool deleteProviderAccount(const QString &accountId);
    ProviderAccount getProviderAccount(const QString &accountId) const;
    ProviderAccountList getAllProviderAccounts() const;
    
    // Prompt operations
    bool createPrompt(const Prompt &prompt);
    bool updatePrompt(const Prompt &prompt);
    bool deletePrompt(const QString &promptId);
    Prompt getPrompt(const QString &promptId) const;
    PromptList getAllPrompts() const;
    PromptList getPromptsInCategory(const QString &category) const;
    
    // Statistics and utilities
    int getConversationMessageCount(const QString &conversationId) const;
    qint64 getTotalStorageSize() const;
    bool cleanupOldData(int daysToKeep = 365);

signals:
    void conversationCreated(const QString &conversationId);
    void conversationUpdated(const QString &conversationId);
    void conversationDeleted(const QString &conversationId);
    void messageCreated(const QString &messageId);
    void messageUpdated(const QString &messageId);
    void messageDeleted(const QString &messageId);

private:
    bool createTables();
    bool runMigrations();
    QSqlQuery prepareQuery(const QString &queryString) const;
    bool executeQuery(QSqlQuery &query) const;
    
    // Migration helpers
    bool migration_001_initial_schema();
    bool migration_002_add_provider_accounts();
    bool migration_003_add_prompts();
    bool migration_004_add_attachments();
    bool migration_005_add_soft_delete_and_sort();
    
    QSqlDatabase &m_database;
    int m_currentVersion;
    static const int LATEST_VERSION = 5;
};

} // namespace GadAI
