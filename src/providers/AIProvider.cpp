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
#include <QUuid>

namespace DesktopApp {

BackendAIProvider::BackendAIProvider(QObject *parent)
    : AIProvider(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl("https://web-production-d7d37.up.railway.app/query")
    , m_currentReply(nullptr)
    , m_currentModel("default")
    , m_status(Status::Disconnected)
    , m_statusMessage("Disconnected")
{
}

QIcon BackendAIProvider::icon() const
{
    return QIcon(":/icons/ai"); // Fallback icon
}

AIProvider::Capabilities BackendAIProvider::capabilities() const
{
    return Capability::TextGeneration | Capability::Streaming;
}

QStringList BackendAIProvider::supportedModels() const
{
    return {"default", "gpt-4", "gpt-3.5-turbo"};
}

QString BackendAIProvider::defaultModel() const
{
    return "default";
}

QString BackendAIProvider::statusMessage() const
{
    return m_statusMessage;
}

QJsonObject BackendAIProvider::defaultConfig() const
{
    QJsonObject config;
    config["token"] = "";
    return config;
}

bool BackendAIProvider::validateConfig(const QJsonObject &config) const
{
    return config.contains("token") && !config["token"].toString().isEmpty();
}

QWidget* BackendAIProvider::createConfigWidget(QWidget *parent)
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
    
    QObject::connect(tokenEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        setAuthToken(text);
    });
    
    return widget;
}

void BackendAIProvider::connect(const QJsonObject &config)
{
    if (config.contains("token")) {
        setAuthToken(config["token"].toString());
        m_status = Status::Connected;
        m_statusMessage = "Connected to Backend AI";
    } else {
        m_status = Status::Error;
        m_statusMessage = "Missing authentication token";
    }
    emit statusChanged(m_status, m_statusMessage);
}

void BackendAIProvider::disconnect()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_status = Status::Disconnected;
    m_statusMessage = "Disconnected";
    emit statusChanged(m_status, m_statusMessage);
}

void BackendAIProvider::setAuthToken(const QString &token)
{
    m_authToken = token;
    if (token.isEmpty()) {
        m_status = Status::Disconnected;
        m_statusMessage = "No authentication token";
    } else {
        m_status = Status::Connected;
        m_statusMessage = "Token configured";
    }
    emit statusChanged(m_status, m_statusMessage);
}

void BackendAIProvider::sendMessage(
    const QString &conversationId,
    const QString &message,
    const QList<Attachment> &attachments,
    const QJsonObject &options)
{
    Q_UNUSED(attachments)
    Q_UNUSED(options)
    
    if (m_authToken.isEmpty()) {
        emit messageFailed(conversationId, "", "No authentication token configured");
        return;
    }

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }

    m_currentConversationId = conversationId;
    m_currentMessageId = QUuid::createUuid().toString();
    m_status = Status::Connecting;
    m_statusMessage = "Sending message...";
    emit statusChanged(m_status, m_statusMessage);
    emit messageStarted(conversationId, m_currentMessageId);

    QNetworkRequest request{QUrl(m_baseUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());

    QJsonObject payload;
    payload["query"] = message;

    QJsonDocument doc(payload);
    
    m_currentReply = m_networkManager->post(request, doc.toJson());
    
    QObject::connect(m_currentReply, &QNetworkReply::finished, this, &BackendAIProvider::onReplyFinished);
    QObject::connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &BackendAIProvider::onError);
}

void BackendAIProvider::regenerateResponse(const QString &conversationId, const QString &messageId)
{
    // For now, just retry the last message
    Q_UNUSED(messageId)
    // Implementation would need to store the last user message to regenerate
    qDebug() << "Regenerate response requested for conversation:" << conversationId;
}

void BackendAIProvider::stopGeneration(const QString &conversationId)
{
    Q_UNUSED(conversationId)
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        m_status = Status::Connected;
        m_statusMessage = "Generation stopped";
        emit statusChanged(m_status, m_statusMessage);
    }
}

void BackendAIProvider::setModel(const QString &model)
{
    m_currentModel = model;
    emit modelChanged(model);
}

QString BackendAIProvider::currentModel() const
{
    return m_currentModel;
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
        message.text = responseText;
        message.createdAt = QDateTime::currentDateTime();
        
        m_status = Status::Connected;
        m_statusMessage = "Response received";
        emit statusChanged(m_status, m_statusMessage);
        emit messageCompleted(m_currentConversationId, m_currentMessageId, message);
    } else {
        m_status = Status::Error;
        m_statusMessage = reply->errorString();
        emit statusChanged(m_status, m_statusMessage);
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
        m_statusMessage = errorMsg;
        emit statusChanged(m_status, m_statusMessage);
        emit messageFailed(m_currentConversationId, m_currentMessageId, errorMsg);
    }
}

} // namespace DesktopApp
