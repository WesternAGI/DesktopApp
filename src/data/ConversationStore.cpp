#include "ConversationStore.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QJsonDocument>
#include <QDebug>
#include <QSqlRecord>

namespace DesktopApp {

ConversationStore::ConversationStore(QSqlDatabase &database, QObject *parent)
    : QObject(parent)
    , m_database(database)
    , m_currentVersion(0)
{
}

bool ConversationStore::initialize()
{
    if (!m_database.isOpen()) {
        qCritical() << "Database is not open";
        return false;
    }
    
    // Create tables if they don't exist
    if (!createTables()) {
        qCritical() << "Failed to create database tables";
        return false;
    }
    
    // Run migrations
    if (!runMigrations()) {
        qCritical() << "Failed to run database migrations";
        return false;
    }
    
    qDebug() << "ConversationStore initialized successfully, version:" << m_currentVersion;
    return true;
}

bool ConversationStore::createTables()
{
    // Check current schema version
    QSqlQuery versionQuery(m_database);
    versionQuery.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='schema_version'");
    
    if (!versionQuery.exec()) {
        qWarning() << "Failed to check schema version:" << versionQuery.lastError().text();
        return false;
    }
    
    if (!versionQuery.next()) {
        // Create schema version table
        QSqlQuery createVersionTable(m_database);
        createVersionTable.prepare(R"(
            CREATE TABLE schema_version (
                version INTEGER PRIMARY KEY
            )
        )");
        
        if (!createVersionTable.exec()) {
            qCritical() << "Failed to create schema_version table:" << createVersionTable.lastError().text();
            return false;
        }
        
        // Insert initial version
        QSqlQuery insertVersion(m_database);
        insertVersion.prepare("INSERT INTO schema_version (version) VALUES (0)");
        if (!insertVersion.exec()) {
            qCritical() << "Failed to insert initial version:" << insertVersion.lastError().text();
            return false;
        }
        
        m_currentVersion = 0;
    } else {
        // Get current version
        QSqlQuery getCurrentVersion(m_database);
        getCurrentVersion.prepare("SELECT version FROM schema_version LIMIT 1");
        if (getCurrentVersion.exec() && getCurrentVersion.next()) {
            m_currentVersion = getCurrentVersion.value(0).toInt();
        }
    }
    
    return true;
}

bool ConversationStore::runMigrations()
{
    qDebug() << "Running migrations from version" << m_currentVersion << "to" << LATEST_VERSION;
    
    while (m_currentVersion < LATEST_VERSION) {
        bool success = false;
        
        qDebug() << "Executing migration for version" << m_currentVersion;
        
        switch (m_currentVersion) {
        case 0:
            qDebug() << "Running migration_001_initial_schema";
            success = migration_001_initial_schema();
            break;
        case 1:
            qDebug() << "Running migration_002_add_provider_accounts";
            success = migration_002_add_provider_accounts();
            break;
        case 2:
            qDebug() << "Running migration_003_add_prompts";
            success = migration_003_add_prompts();
            break;
        case 3:
            qDebug() << "Running migration_004_add_attachments";
            success = migration_004_add_attachments();
            break;
        case 4:
            qDebug() << "Running migration_005_add_soft_delete_and_sort";
            success = migration_005_add_soft_delete_and_sort();
            break;
        default:
            qCritical() << "Unknown migration version:" << m_currentVersion;
            return false;
        }
        
        if (!success) {
            qCritical() << "Migration failed at version:" << m_currentVersion;
            return false;
        }
        
        m_currentVersion++;
        
        // Update version in database
        QSqlQuery updateVersion(m_database);
        updateVersion.prepare("UPDATE schema_version SET version = ?");
        updateVersion.addBindValue(m_currentVersion);
        
        if (!updateVersion.exec()) {
            qCritical() << "Failed to update schema version:" << updateVersion.lastError().text();
            return false;
        }
        
        qDebug() << "Migration to version" << m_currentVersion << "completed";
    }
    
    return true;
}

bool ConversationStore::migration_001_initial_schema()
{
    qDebug() << "Running migration 001: Initial schema";
    
    // Create conversations table
    QSqlQuery createConversations(m_database);
    createConversations.prepare(R"(
        CREATE TABLE conversations (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            pinned INTEGER DEFAULT 0,
            archived INTEGER DEFAULT 0,
            provider_id TEXT DEFAULT 'echo',
            model_name TEXT DEFAULT 'echo-model',
            metadata TEXT DEFAULT '{}'
        )
    )");
    
    if (!createConversations.exec()) {
        qCritical() << "Failed to create conversations table:" << createConversations.lastError().text();
        return false;
    }
    
    // Create messages table
    QSqlQuery createMessages(m_database);
    createMessages.prepare(R"(
        CREATE TABLE messages (
            id TEXT PRIMARY KEY,
            conversation_id TEXT NOT NULL,
            role TEXT NOT NULL,
            text TEXT NOT NULL,
            created_at TEXT NOT NULL,
            metadata TEXT DEFAULT '{}',
            parent_id TEXT,
            is_streaming INTEGER DEFAULT 0,
            FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE
        )
    )");
    
    if (!createMessages.exec()) {
        qCritical() << "Failed to create messages table:" << createMessages.lastError().text();
        return false;
    }
    
    // Create indexes for performance
    QStringList indexes = {
        "CREATE INDEX idx_conversations_updated_at ON conversations(updated_at DESC)",
        "CREATE INDEX idx_conversations_pinned ON conversations(pinned, updated_at DESC)",
        "CREATE INDEX idx_messages_conversation_id ON messages(conversation_id, created_at)",
        "CREATE INDEX idx_messages_role ON messages(role)"
    };
    
    for (const QString &indexSql : indexes) {
        QSqlQuery createIndex(m_database);
        createIndex.prepare(indexSql);
        if (!createIndex.exec()) {
            qWarning() << "Failed to create index:" << createIndex.lastError().text();
        }
    }
    
    return true;
}

bool ConversationStore::migration_002_add_provider_accounts()
{
    qDebug() << "Running migration 002: Add provider accounts";
    
    QSqlQuery createProviderAccounts(m_database);
    createProviderAccounts.prepare(R"(
        CREATE TABLE provider_accounts (
            id TEXT PRIMARY KEY,
            provider TEXT NOT NULL,
            label TEXT NOT NULL,
            endpoint TEXT,
            api_key_ref TEXT,
            default_model TEXT,
            parameters TEXT DEFAULT '{}',
            enabled INTEGER DEFAULT 1,
            created_at TEXT NOT NULL
        )
    )");
    
    if (!createProviderAccounts.exec()) {
        qCritical() << "Failed to create provider_accounts table:" << createProviderAccounts.lastError().text();
        return false;
    }
    
    // Insert default echo provider
    QSqlQuery insertEcho(m_database);
    insertEcho.prepare(R"(
        INSERT INTO provider_accounts (id, provider, label, default_model, created_at)
        VALUES ('echo-default', 'echo', 'Echo Provider', 'echo-model', datetime('now'))
    )");
    
    if (!insertEcho.exec()) {
        qWarning() << "Failed to insert default echo provider:" << insertEcho.lastError().text();
    }
    
    return true;
}

bool ConversationStore::migration_003_add_prompts()
{
    qDebug() << "Running migration 003: Add prompts";
    
    QSqlQuery createPrompts(m_database);
    createPrompts.prepare(R"(
        CREATE TABLE prompts (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            text TEXT NOT NULL,
            variables TEXT DEFAULT '{}',
            category TEXT DEFAULT 'General',
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        )
    )");
    
    if (!createPrompts.exec()) {
        qCritical() << "Failed to create prompts table:" << createPrompts.lastError().text();
        return false;
    }
    
    // Insert some default prompts
    QStringList defaultPrompts = {
        "('default-explain', 'Explain Simply', 'Explain {{topic}} in simple terms that a beginner could understand.', '{\"topic\":\"\"}', 'Education', datetime('now'), datetime('now'))",
        "('default-summarize', 'Summarize Text', 'Please summarize the following text: {{text}}', '{\"text\":\"\"}', 'Productivity', datetime('now'), datetime('now'))",
        "('default-creative', 'Creative Writing', 'Write a creative story about {{subject}}.', '{\"subject\":\"\"}', 'Creative', datetime('now'), datetime('now'))"
    };
    
    for (const QString &promptData : defaultPrompts) {
        QSqlQuery insertPrompt(m_database);
        insertPrompt.prepare("INSERT INTO prompts (id, name, text, variables, category, created_at, updated_at) VALUES " + promptData);
        if (!insertPrompt.exec()) {
            qWarning() << "Failed to insert default prompt:" << insertPrompt.lastError().text();
        }
    }
    
    return true;
}

bool ConversationStore::migration_004_add_attachments()
{
    qDebug() << "Running migration 004: Add attachments";
    
    QSqlQuery createAttachments(m_database);
    createAttachments.prepare(R"(
        CREATE TABLE attachments (
            id TEXT PRIMARY KEY,
            message_id TEXT NOT NULL,
            type TEXT NOT NULL,
            file_name TEXT NOT NULL,
            file_path TEXT NOT NULL,
            mime_type TEXT,
            file_size INTEGER DEFAULT 0,
            created_at TEXT NOT NULL,
            metadata TEXT DEFAULT '{}',
            FOREIGN KEY (message_id) REFERENCES messages(id) ON DELETE CASCADE
        )
    )");
    
    if (!createAttachments.exec()) {
        qCritical() << "Failed to create attachments table:" << createAttachments.lastError().text();
        return false;
    }
    
    // Create index for message attachments
    QSqlQuery createAttachmentIndex(m_database);
    createAttachmentIndex.prepare("CREATE INDEX idx_attachments_message_id ON attachments(message_id)");
    if (!createAttachmentIndex.exec()) {
        qWarning() << "Failed to create attachment index:" << createAttachmentIndex.lastError().text();
    }
    
    return true;
}

bool ConversationStore::migration_005_add_soft_delete_and_sort()
{
    qDebug() << "Running migration 005: Add soft delete and sort order";
    
    QSqlQuery alter1(m_database);
    alter1.prepare("ALTER TABLE conversations ADD COLUMN deleted INTEGER DEFAULT 0");
    if (!alter1.exec()) {
        qWarning() << "Migration 005: deleted column error:" << alter1.lastError().text();
    } else {
        qDebug() << "Migration 005: Added deleted column successfully";
    }
    
    QSqlQuery alter2(m_database);
    alter2.prepare("ALTER TABLE conversations ADD COLUMN sort_order INTEGER DEFAULT 0");
    if (!alter2.exec()) {
        qWarning() << "Migration 005: sort_order column error:" << alter2.lastError().text();
    } else {
        qDebug() << "Migration 005: Added sort_order column successfully";
    }
    
    // Verify columns were added
    QSqlQuery verify(m_database);
    verify.prepare("PRAGMA table_info(conversations)");
    if (verify.exec()) {
        qDebug() << "Post-migration column list:";
        while (verify.next()) {
            qDebug() << "  Column:" << verify.value("name").toString() << "Type:" << verify.value("type").toString();
        }
    }
    
    return true;
}

// Conversation operations
bool ConversationStore::createConversation(const Conversation &conversation)
{
    QSqlQuery query(m_database);
    
    // Try the full INSERT first (11 columns)
    qDebug() << "Attempting full INSERT with 11 columns";
    query.prepare(R"(
        INSERT INTO conversations (id, title, created_at, updated_at, pinned, archived, provider_id, model_name, metadata, deleted, sort_order)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(conversation.id);
    query.addBindValue(conversation.title);
    query.addBindValue(conversation.createdAt.toString(Qt::ISODate));
    query.addBindValue(conversation.updatedAt.toString(Qt::ISODate));
    query.addBindValue(conversation.pinned ? 1 : 0);
    query.addBindValue(conversation.archived ? 1 : 0);
    query.addBindValue(conversation.providerId);
    query.addBindValue(conversation.modelName);
    query.addBindValue(QJsonDocument(conversation.metadata).toJson(QJsonDocument::Compact));
    query.addBindValue(conversation.deleted ? 1 : 0);
    query.addBindValue(conversation.sortOrder);
    
    if (query.exec()) {
        qDebug() << "Full INSERT succeeded";
        emit conversationCreated(conversation.id);
        return true;
    }
    
    qWarning() << "Full INSERT failed:" << query.lastError().text();
    
    // Fall back to legacy INSERT (9 columns)
    qDebug() << "Attempting legacy INSERT with 9 columns";
    query.clear();
    query.prepare(R"(
        INSERT INTO conversations (id, title, created_at, updated_at, pinned, archived, provider_id, model_name, metadata)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(conversation.id);
    query.addBindValue(conversation.title);
    query.addBindValue(conversation.createdAt.toString(Qt::ISODate));
    query.addBindValue(conversation.updatedAt.toString(Qt::ISODate));
    query.addBindValue(conversation.pinned ? 1 : 0);
    query.addBindValue(conversation.archived ? 1 : 0);
    query.addBindValue(conversation.providerId);
    query.addBindValue(conversation.modelName);
    query.addBindValue(QJsonDocument(conversation.metadata).toJson(QJsonDocument::Compact));
    
    if (!query.exec()) {
        qWarning() << "Failed to create conversation:" << query.lastError().text();
        qWarning() << "Query:" << query.lastQuery();
        return false;
    }
    
    qDebug() << "Legacy INSERT succeeded";
    emit conversationCreated(conversation.id);
    return true;
}

bool ConversationStore::updateConversation(const Conversation &conversation)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE conversations 
        SET title = ?, updated_at = ?, pinned = ?, archived = ?, provider_id = ?, model_name = ?, metadata = ?, deleted = ?, sort_order = ?
        WHERE id = ?
    )");
    
    query.addBindValue(conversation.title);
    query.addBindValue(conversation.updatedAt.toString(Qt::ISODate));
    query.addBindValue(conversation.pinned ? 1 : 0);
    query.addBindValue(conversation.archived ? 1 : 0);
    query.addBindValue(conversation.providerId);
    query.addBindValue(conversation.modelName);
    query.addBindValue(QJsonDocument(conversation.metadata).toJson(QJsonDocument::Compact));
    query.addBindValue(conversation.deleted ? 1 : 0);
    query.addBindValue(conversation.sortOrder);
    query.addBindValue(conversation.id);
    
    if (!query.exec()) {
        qWarning() << "Failed to update conversation:" << query.lastError().text();
        return false;
    }
    
    emit conversationUpdated(conversation.id);
    return true;
}

bool ConversationStore::deleteConversation(const QString &conversationId)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM conversations WHERE id = ?");
    query.addBindValue(conversationId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete conversation:" << query.lastError().text();
        return false;
    }
    
    emit conversationDeleted(conversationId);
    return true;
}

Conversation ConversationStore::getConversation(const QString &conversationId) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE id = ?");
    query.addBindValue(conversationId);
    
    if (!query.exec() || !query.next()) {
        return Conversation(); // Invalid conversation
    }
    
    Conversation conv;
    conv.id = query.value("id").toString();
    conv.title = query.value("title").toString();
    conv.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    conv.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    conv.pinned = query.value("pinned").toBool();
    conv.archived = query.value("archived").toBool();
    QSqlRecord rec = query.record();
    int delIdx = rec.indexOf("deleted");
    int sortIdx = rec.indexOf("sort_order");
    conv.deleted = delIdx >= 0 ? query.value(delIdx).toBool() : false;
    conv.sortOrder = sortIdx >= 0 ? query.value(sortIdx).toInt() : 0;
    conv.providerId = query.value("provider_id").toString();
    conv.modelName = query.value("model_name").toString();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(query.value("metadata").toByteArray(), &error);
    if (error.error == QJsonParseError::NoError) {
        conv.metadata = doc.object();
    }
    
    return conv;
}

ConversationList ConversationStore::getAllConversations() const
{
    return getRecentConversations(1000); // Get all with a reasonable limit
}

ConversationList ConversationStore::getRecentConversations(int limit) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE archived = 0 AND (deleted IS NULL OR deleted = 0) ORDER BY pinned DESC, sort_order ASC, updated_at DESC LIMIT ?");
    query.addBindValue(limit);
    
    ConversationList conversations;
    
    if (!query.exec()) {
        qWarning() << "Failed to get recent conversations:" << query.lastError().text();
        return conversations;
    }
    
    while (query.next()) {
        Conversation conv;
        conv.id = query.value("id").toString();
        conv.title = query.value("title").toString();
        conv.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        conv.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    conv.pinned = query.value("pinned").toBool();
    conv.archived = query.value("archived").toBool();
    QSqlRecord r = query.record();
    int deletedIdx = r.indexOf("deleted");
    if (deletedIdx>=0) conv.deleted = query.value(deletedIdx).toBool();
    int sortIdx = r.indexOf("sort_order");
    if (sortIdx>=0) conv.sortOrder = query.value(sortIdx).toInt();
        conv.providerId = query.value("provider_id").toString();
        conv.modelName = query.value("model_name").toString();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(query.value("metadata").toByteArray(), &error);
        if (error.error == QJsonParseError::NoError) {
            conv.metadata = doc.object();
        }
        
        conversations.append(conv);
    }
    
    return conversations;
}

ConversationList ConversationStore::getPinnedConversations(int limit) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE pinned = 1 AND (deleted IS NULL OR deleted = 0) ORDER BY sort_order ASC, updated_at DESC LIMIT ?");
    query.addBindValue(limit);
    ConversationList list;
    if (query.exec()) {
        while (query.next()) {
            Conversation conv;
            conv.id = query.value("id").toString();
            conv.title = query.value("title").toString();
            conv.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
            conv.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
            conv.pinned = true;
            conv.archived = query.value("archived").toBool();
            QSqlRecord r = query.record();
            int delIdx = r.indexOf("deleted"); if (delIdx>=0) conv.deleted = query.value(delIdx).toBool();
            int sortIdx = r.indexOf("sort_order"); if (sortIdx>=0) conv.sortOrder = query.value(sortIdx).toInt();
            list.append(conv);
        }
    }
    return list;
}

ConversationList ConversationStore::getArchivedConversations(int limit) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE archived = 1 AND (deleted IS NULL OR deleted = 0) ORDER BY updated_at DESC LIMIT ?");
    query.addBindValue(limit);
    ConversationList list;
    if (query.exec()) {
        while (query.next()) {
            Conversation conv;
            conv.id = query.value("id").toString();
            conv.title = query.value("title").toString();
            conv.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
            conv.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
            conv.pinned = query.value("pinned").toBool();
            conv.archived = true;
            QSqlRecord r = query.record();
            int delIdx = r.indexOf("deleted"); if (delIdx>=0) conv.deleted = query.value(delIdx).toBool();
            int sortIdx = r.indexOf("sort_order"); if (sortIdx>=0) conv.sortOrder = query.value(sortIdx).toInt();
            list.append(conv);
        }
    }
    return list;
}

ConversationList ConversationStore::getTrashConversations(int limit) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE deleted = 1 ORDER BY updated_at DESC LIMIT ?");
    query.addBindValue(limit);
    ConversationList list;
    if (query.exec()) {
        while (query.next()) {
            Conversation conv;
            conv.id = query.value("id").toString();
            conv.title = query.value("title").toString();
            conv.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
            conv.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
            conv.pinned = query.value("pinned").toBool();
            conv.archived = query.value("archived").toBool();
            conv.deleted = true;
            QSqlRecord r = query.record();
            int sortIdx = r.indexOf("sort_order"); if (sortIdx>=0) conv.sortOrder = query.value(sortIdx).toInt();
            list.append(conv);
        }
    }
    return list;
}

// Message operations
bool ConversationStore::createMessage(const Message &message)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO messages (id, conversation_id, role, text, created_at, metadata, parent_id, is_streaming)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(message.id);
    query.addBindValue(message.conversationId);
    query.addBindValue(messageRoleToString(message.role));
    query.addBindValue(message.text);
    query.addBindValue(message.createdAt.toString(Qt::ISODate));
    query.addBindValue(QJsonDocument(message.metadata).toJson(QJsonDocument::Compact));
    query.addBindValue(message.parentId);
    query.addBindValue(message.isStreaming ? 1 : 0);
    
    if (!query.exec()) {
        qWarning() << "Failed to create message:" << query.lastError().text();
        return false;
    }
    
    emit messageCreated(message.id);
    return true;
}

bool ConversationStore::updateMessage(const Message &message)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE messages
        SET text = ?, metadata = ?, parent_id = ?, is_streaming = ?
        WHERE id = ?
    )");
    query.addBindValue(message.text);
    query.addBindValue(QJsonDocument(message.metadata).toJson(QJsonDocument::Compact));
    query.addBindValue(message.parentId);
    query.addBindValue(message.isStreaming ? 1 : 0);
    query.addBindValue(message.id);
    if (!query.exec()) {
        qWarning() << "Failed to update message:" << query.lastError().text();
        return false;
    }
    emit messageUpdated(message.id);
    return true;
}

bool ConversationStore::deleteMessage(const QString &messageId)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM messages WHERE id = ?");
    query.addBindValue(messageId);
    if (!query.exec()) {
        qWarning() << "Failed to delete message:" << query.lastError().text();
        return false;
    }
    emit messageDeleted(messageId);
    return true;
}

MessageList ConversationStore::getMessagesForConversation(const QString &conversationId) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? ORDER BY created_at ASC");
    query.addBindValue(conversationId);
    
    MessageList messages;
    
    if (!query.exec()) {
        qWarning() << "Failed to get messages for conversation:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        Message msg;
        msg.id = query.value("id").toString();
        msg.conversationId = query.value("conversation_id").toString();
        msg.role = messageRoleFromString(query.value("role").toString());
        msg.text = query.value("text").toString();
        msg.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        msg.parentId = query.value("parent_id").toString();
        msg.isStreaming = query.value("is_streaming").toBool();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(query.value("metadata").toByteArray(), &error);
        if (error.error == QJsonParseError::NoError) {
            msg.metadata = doc.object();
        }
        
        messages.append(msg);
    }
    
    return messages;
}

Message ConversationStore::getMessage(const QString &messageId) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE id = ? LIMIT 1");
    query.addBindValue(messageId);
    if (!query.exec() || !query.next()) {
        return Message();
    }
    Message msg;
    msg.id = query.value("id").toString();
    msg.conversationId = query.value("conversation_id").toString();
    msg.role = messageRoleFromString(query.value("role").toString());
    msg.text = query.value("text").toString();
    msg.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    msg.parentId = query.value("parent_id").toString();
    msg.isStreaming = query.value("is_streaming").toBool();
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(query.value("metadata").toByteArray(), &error);
    if (error.error == QJsonParseError::NoError) {
        msg.metadata = doc.object();
    }
    return msg;
}

MessageList ConversationStore::getRecentMessages(const QString &conversationId, int limit) const
{
    MessageList messages;
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM messages
        WHERE conversation_id = ?
        ORDER BY datetime(created_at) DESC
        LIMIT ?
    )");
    query.addBindValue(conversationId);
    query.addBindValue(limit);
    if (!query.exec()) {
        qWarning() << "Failed to get recent messages:" << query.lastError().text();
        return messages;
    }
    while (query.next()) {
        Message msg;
        msg.id = query.value("id").toString();
        msg.conversationId = query.value("conversation_id").toString();
        msg.role = messageRoleFromString(query.value("role").toString());
        msg.text = query.value("text").toString();
        msg.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        QJsonParseError parseError;
        QJsonDocument metaDoc = QJsonDocument::fromJson(query.value("metadata").toByteArray(), &parseError);
        if (parseError.error == QJsonParseError::NoError && metaDoc.isObject()) {
            msg.metadata = metaDoc.object();
        }
        msg.parentId = query.value("parent_id").toString();
        msg.isStreaming = query.value("is_streaming").toBool();
        messages.append(msg);
    }
    // Return in chronological order (oldest first)
    std::reverse(messages.begin(), messages.end());
    return messages;
}

// Utility methods
int ConversationStore::getConversationMessageCount(const QString &conversationId) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QSqlQuery ConversationStore::prepareQuery(const QString &queryString) const
{
    QSqlQuery query(m_database);
    query.prepare(queryString);
    return query;
}

bool ConversationStore::executeQuery(QSqlQuery &query) const
{
    if (!query.exec()) {
        qWarning() << "Query execution failed:" << query.lastError().text();
        qWarning() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

} // namespace DesktopApp
