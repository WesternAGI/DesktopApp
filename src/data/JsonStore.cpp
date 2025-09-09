#include "JsonStore.h"
#include <QFile>
#include <QJsonParseError>
#include <QDebug>

namespace GadAI {

JsonStore::JsonStore(QObject *parent)
    : QObject(parent)
    , m_autoSaveTimer(new QTimer(this))
{
    m_autoSaveTimer->setSingleShot(true);
    m_autoSaveTimer->setInterval(1000); // Auto-save after 1 second of inactivity
    connect(m_autoSaveTimer, &QTimer::timeout, this, &JsonStore::saveData);
}

JsonStore::~JsonStore()
{
    if (m_loaded) {
        saveData();
    }
}

bool JsonStore::initialize(const QString &dataDir)
{
    m_dataDir = dataDir;
    m_conversationsFile = QDir(dataDir).filePath("conversations.json");
    m_messagesFile = QDir(dataDir).filePath("messages.json");
    
    // Ensure data directory exists
    QDir dir;
    if (!dir.mkpath(dataDir)) {
        qCritical() << "Failed to create data directory:" << dataDir;
        return false;
    }
    
    loadData();
    m_loaded = true;
    qDebug() << "JsonStore initialized with" << m_conversations.size() << "conversations and" << m_messages.size() << "messages";
    return true;
}

void JsonStore::loadData()
{
    // Load conversations
    QFile convFile(m_conversationsFile);
    if (convFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(convFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            m_conversations = doc.object();
        }
        convFile.close();
    }
    
    // Load messages
    QFile msgFile(m_messagesFile);
    if (msgFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(msgFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            m_messages = doc.object();
        }
        msgFile.close();
    }
}

void JsonStore::saveData()
{
    // Save conversations
    QFile convFile(m_conversationsFile);
    if (convFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_conversations);
        convFile.write(doc.toJson());
        convFile.close();
    }
    
    // Save messages
    QFile msgFile(m_messagesFile);
    if (msgFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_messages);
        msgFile.write(doc.toJson());
        msgFile.close();
    }
}

void JsonStore::scheduleAutoSave()
{
    if (m_loaded) {
        m_autoSaveTimer->start();
    }
}

// Conversation operations
bool JsonStore::createConversation(const Conversation &conversation)
{
    QJsonObject convObj = conversation.toJson();
    m_conversations[conversation.id] = convObj;
    scheduleAutoSave();
    emit conversationCreated(conversation.id);
    return true;
}

bool JsonStore::updateConversation(const Conversation &conversation)
{
    if (!m_conversations.contains(conversation.id)) {
        return false;
    }
    
    QJsonObject convObj = conversation.toJson();
    m_conversations[conversation.id] = convObj;
    scheduleAutoSave();
    emit conversationUpdated(conversation.id);
    return true;
}

bool JsonStore::deleteConversation(const QString &conversationId)
{
    if (!m_conversations.contains(conversationId)) {
        return false;
    }
    
    m_conversations.remove(conversationId);
    
    // Remove associated messages
    QStringList messageIds;
    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        QJsonObject msgObj = it.value().toObject();
        if (msgObj["conversationId"].toString() == conversationId) {
            messageIds.append(it.key());
        }
    }
    
    for (const QString &msgId : messageIds) {
        m_messages.remove(msgId);
    }
    
    scheduleAutoSave();
    emit conversationDeleted(conversationId);
    return true;
}

Conversation JsonStore::getConversation(const QString &conversationId) const
{
    if (!m_conversations.contains(conversationId)) {
        return Conversation(); // Invalid
    }
    
    QJsonObject convObj = m_conversations[conversationId].toObject();
    return Conversation::fromJson(convObj);
}

ConversationList JsonStore::getAllConversations() const
{
    ConversationList list;
    for (auto it = m_conversations.begin(); it != m_conversations.end(); ++it) {
        QJsonObject convObj = it.value().toObject();
        Conversation conv = Conversation::fromJson(convObj);
        if (conv.isValid()) {
            list.append(conv);
        }
    }
    return list;
}

ConversationList JsonStore::getRecentConversations(int limit) const
{
    ConversationList all = getAllConversations();
    
    // Filter non-archived, non-deleted
    ConversationList filtered;
    for (const Conversation &conv : all) {
        if (!conv.archived && !conv.deleted) {
            filtered.append(conv);
        }
    }
    
    // Sort by pinned first, then by updated time
    std::sort(filtered.begin(), filtered.end(), [](const Conversation &a, const Conversation &b) {
        if (a.pinned != b.pinned) return a.pinned > b.pinned;
        return a.updatedAt > b.updatedAt;
    });
    
    if (limit > 0 && filtered.size() > limit) {
        filtered = filtered.mid(0, limit);
    }
    
    return filtered;
}

ConversationList JsonStore::getPinnedConversations(int limit) const
{
    ConversationList all = getAllConversations();
    
    ConversationList filtered;
    for (const Conversation &conv : all) {
        if (conv.pinned && !conv.deleted) {
            filtered.append(conv);
        }
    }
    
    std::sort(filtered.begin(), filtered.end(), [](const Conversation &a, const Conversation &b) {
        if (a.sortOrder != b.sortOrder) return a.sortOrder < b.sortOrder;
        return a.updatedAt > b.updatedAt;
    });
    
    if (limit > 0 && filtered.size() > limit) {
        filtered = filtered.mid(0, limit);
    }
    
    return filtered;
}

ConversationList JsonStore::getArchivedConversations(int limit) const
{
    ConversationList all = getAllConversations();
    
    ConversationList filtered;
    for (const Conversation &conv : all) {
        if (conv.archived && !conv.deleted) {
            filtered.append(conv);
        }
    }
    
    std::sort(filtered.begin(), filtered.end(), [](const Conversation &a, const Conversation &b) {
        return a.updatedAt > b.updatedAt;
    });
    
    if (limit > 0 && filtered.size() > limit) {
        filtered = filtered.mid(0, limit);
    }
    
    return filtered;
}

ConversationList JsonStore::getTrashConversations(int limit) const
{
    ConversationList all = getAllConversations();
    
    ConversationList filtered;
    for (const Conversation &conv : all) {
        if (conv.deleted) {
            filtered.append(conv);
        }
    }
    
    std::sort(filtered.begin(), filtered.end(), [](const Conversation &a, const Conversation &b) {
        return a.updatedAt > b.updatedAt;
    });
    
    if (limit > 0 && filtered.size() > limit) {
        filtered = filtered.mid(0, limit);
    }
    
    return filtered;
}

// Message operations
bool JsonStore::createMessage(const Message &message)
{
    QJsonObject msgObj = message.toJson();
    m_messages[message.id] = msgObj;
    scheduleAutoSave();
    emit messageCreated(message.id);
    return true;
}

bool JsonStore::updateMessage(const Message &message)
{
    if (!m_messages.contains(message.id)) {
        return false;
    }
    
    QJsonObject msgObj = message.toJson();
    m_messages[message.id] = msgObj;
    scheduleAutoSave();
    emit messageUpdated(message.id);
    return true;
}

bool JsonStore::deleteMessage(const QString &messageId)
{
    if (!m_messages.contains(messageId)) {
        return false;
    }
    
    m_messages.remove(messageId);
    scheduleAutoSave();
    emit messageDeleted(messageId);
    return true;
}

Message JsonStore::getMessage(const QString &messageId) const
{
    if (!m_messages.contains(messageId)) {
        return Message(); // Invalid
    }
    
    QJsonObject msgObj = m_messages[messageId].toObject();
    return Message::fromJson(msgObj);
}

MessageList JsonStore::getMessagesForConversation(const QString &conversationId) const
{
    MessageList list;
    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        QJsonObject msgObj = it.value().toObject();
        if (msgObj["conversationId"].toString() == conversationId) {
            Message msg = Message::fromJson(msgObj);
            if (msg.isValid()) {
                list.append(msg);
            }
        }
    }
    
    // Sort by creation time
    std::sort(list.begin(), list.end(), [](const Message &a, const Message &b) {
        return a.createdAt < b.createdAt;
    });
    
    return list;
}

int JsonStore::getConversationMessageCount(const QString &conversationId) const
{
    int count = 0;
    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        QJsonObject msgObj = it.value().toObject();
        if (msgObj["conversationId"].toString() == conversationId) {
            count++;
        }
    }
    return count;
}

} // namespace GadAI
