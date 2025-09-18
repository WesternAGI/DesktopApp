#include "AIProvider.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QIcon>

namespace DesktopApp {

BackendAIProvider::BackendAIProvider(QObject *parent)
    : AIProvider(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl("https://web-production-d7d37.up.railway.app/query")
    , m_status(Status::Disconnected)
    , m_currentReply(nullptr)
{
}

QIcon BackendAIProvider::icon() const
{
    return QIcon(":/icons/ai"); // Fallback icon
}

bool BackendAIProvider::isAvailable() const
{
    return !m_authToken.isEmpty() && m_status != Status::Error;
}

QWidget* BackendAIProvider::configWidget(QWidget *parent)
{
    auto widget = new QWidget(parent);
    auto layout = new QVBoxLayout(widget);
    
    layout->addWidget(new QLabel("Backend AI Configuration:", widget));
    
    auto tokenEdit = new QLineEdit(widget);
    tokenEdit->setPlaceholderText("Enter authentication token...");
    tokenEdit->setEchoMode(QLineEdit::Password);
    tokenEdit->setText(m_authToken);
    
    layout->addWidget(new QLabel("Token:", widget));
    layout->addWidget(tokenEdit);
    
    connect(tokenEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        setAuthToken(text);
    });
    
    return widget;
}

bool BackendAIProvider::validateConfig(const QJsonObject &config) const
{
    return config.contains("token") && !config["token"].toString().isEmpty();
}

void BackendAIProvider::connect(const QJsonObject &config)
{
    if (config.contains("token")) {
        setAuthToken(config["token"].toString());
        m_status = Status::Connected;
    } else {
        m_status = Status::Error;
    }
    emit statusChanged(m_status, "Connected to Backend AI");
}

void BackendAIProvider::disconnect()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_status = Status::Disconnected;
    emit statusChanged(m_status, "Disconnected");
}

void BackendAIProvider::setAuthToken(const QString &token)
{
    m_authToken = token;
    m_status = token.isEmpty() ? Status::Disconnected : Status::Connected;
    emit statusChanged(m_status, token.isEmpty() ? "No token" : "Token configured");
}

void BackendAIProvider::sendMessage(const QString &conversationId, 
                                  const QString &messageId, 
                                  const QString &text,
                                  const QList<Attachment> &attachments,
                                  const QJsonObject &options)
{
    Q_UNUSED(attachments)
    Q_UNUSED(options)
    
    if (m_authToken.isEmpty()) {
        emit messageFailed(conversationId, messageId, "No authentication token configured");
        return;
    }

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }

    m_currentConversationId = conversationId;
    m_currentMessageId = messageId;
    m_status = Status::Connecting;
    emit statusChanged(m_status, "Sending message...");
    emit messageStarted(conversationId, messageId);

    QNetworkRequest request(QUrl(m_baseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());

    QJsonObject payload;
    payload["message"] = text;

    QJsonDocument doc(payload);
    
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    connect(m_currentReply, &QNetworkReply::finished, this, &BackendAIProvider::onReplyFinished);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &BackendAIProvider::onError);
}

void BackendAIProvider::onReplyFinished()
{
    if (!m_currentReply) return;

    auto reply = m_currentReply;
    m_currentReply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject response = doc.object();
        
        QString responseText = response["response"].toString();
        if (responseText.isEmpty()) {
            responseText = "Empty response from AI";
        }
        
        // Create a message object
        Message message;
        message.id = m_currentMessageId;
        message.conversationId = m_currentConversationId;
        message.role = MessageRole::Assistant;
        message.content = responseText;
        message.timestamp = QDateTime::currentDateTime();
        
        m_status = Status::Connected;
        emit statusChanged(m_status, "Response received");
        emit messageCompleted(m_currentConversationId, m_currentMessageId, message);
    } else {
        m_status = Status::Error;
        emit statusChanged(m_status, reply->errorString());
        emit messageFailed(m_currentConversationId, m_currentMessageId, reply->errorString());
    }
    
    reply->deleteLater();
}

void BackendAIProvider::onError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    
    if (m_currentReply) {
        QString errorMsg = m_currentReply->errorString();
        m_status = Status::Error;
        emit statusChanged(m_status, errorMsg);
        emit messageFailed(m_currentConversationId, m_currentMessageId, errorMsg);
    }
}

} // namespace DesktopApp
