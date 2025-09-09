#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include "Models.h"

namespace DesktopApp {

/**
 * @brief Lightweight JSON-based storage replacing SQLite
 */
class JsonStore : public QObject
{
    Q_OBJECT

public:
    explicit JsonStore(QObject *parent = nullptr);
    ~JsonStore();

    bool initialize(const QString &dataDir);

    // Conversation operations
    bool createConversation(const Conversation &conversation);
    bool updateConversation(const Conversation &conversation);
    bool deleteConversation(const QString &conversationId);
    Conversation getConversation(const QString &conversationId) const;
    ConversationList getAllConversations() const;
    ConversationList getRecentConversations(int limit = 100) const;
    ConversationList getPinnedConversations(int limit = 100) const;
    ConversationList getArchivedConversations(int limit = 100) const;
    ConversationList getTrashConversations(int limit = 100) const;

    // Message operations
    bool createMessage(const Message &message);
    bool updateMessage(const Message &message);
    bool deleteMessage(const QString &messageId);
    Message getMessage(const QString &messageId) const;
    MessageList getMessagesForConversation(const QString &conversationId) const;
    int getConversationMessageCount(const QString &conversationId) const;

signals:
    void conversationCreated(const QString &conversationId);
    void conversationUpdated(const QString &conversationId);
    void conversationDeleted(const QString &conversationId);
    void messageCreated(const QString &messageId);
    void messageUpdated(const QString &messageId);
    void messageDeleted(const QString &messageId);

private slots:
    void saveData();

private:
    void loadData();
    void scheduleAutoSave();

    QString m_dataDir;
    QString m_conversationsFile;
    QString m_messagesFile;
    
    QJsonObject m_conversations;
    QJsonObject m_messages;
    QTimer *m_autoSaveTimer;
    
    bool m_loaded = false;
};

} // namespace DesktopApp
