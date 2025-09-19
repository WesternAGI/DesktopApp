#include "AIProvider.h"
#include "core/Application.h"
#include "services/AuthenticationService.h"
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
    Q_UNUSED(config)
    
    // Get the user's authentication token from the login session
    auto *app = Application::instance();
    if (!app || !app->authenticationService()) {
        m_status = Status::Error;
        m_statusMessage = "Authentication service not available";
        emit statusChanged(m_status, m_statusMessage);
        return;
    }
    
    QString userToken = app->authenticationService()->getCurrentToken();
    if (userToken.isEmpty()) {
        m_status = Status::Error;
        m_statusMessage = "User not authenticated - please log in";
        emit statusChanged(m_status, m_statusMessage);
        return;
    }
    
    // Use the user's token instead of config token
    setAuthToken(userToken);
    m_status = Status::Connected;
    m_statusMessage = "Connected to Backend AI with user token";
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
    QString authHeader = QString("Bearer %1").arg(m_authToken);
    request.setRawHeader("Authorization", authHeader.toUtf8());
    
    qDebug() << "BackendAIProvider: Making request to" << m_baseUrl;
    qDebug() << "BackendAIProvider: Auth token length:" << m_authToken.length();
    qDebug() << "BackendAIProvider: Auth header:" << authHeader;

    QJsonObject payload;
    payload["query"] = message;
    payload["chat_id"] = conversationId; // Use the conversation ID as chat_id

    QJsonDocument doc(payload);
    qDebug() << "BackendAIProvider: Request payload:" << doc.toJson(QJsonDocument::Compact);
    
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
    
    // Read the raw response data
    QByteArray responseData = reply->readAll();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    qDebug() << "BackendAIProvider: Response received";
    qDebug() << "BackendAIProvider: HTTP Status:" << httpStatus;
    qDebug() << "BackendAIProvider: Response data length:" << responseData.length();
    qDebug() << "BackendAIProvider: Response data:" << responseData;
    qDebug() << "BackendAIProvider: Network error:" << reply->error();
    qDebug() << "BackendAIProvider: Error string:" << reply->errorString();
    
    if (reply->error() == QNetworkReply::NoError && httpStatus == 200) {
        if (responseData.isEmpty()) {
            QString errorMsg = "Server returned empty response (HTTP 200 but no content)";
            qDebug() << "BackendAIProvider:" << errorMsg;
            
            Message message;
            message.id = m_currentMessageId;
            message.conversationId = m_currentConversationId;
            message.role = MessageRole::Assistant;
            message.text = "Error: " + errorMsg;
            message.createdAt = QDateTime::currentDateTime();
            
            m_status = Status::Error;
            m_statusMessage = errorMsg;
            emit statusChanged(m_status, m_statusMessage);
            emit messageCompleted(m_currentConversationId, m_currentMessageId, message);
        } else {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = QString("Failed to parse response JSON: %1").arg(parseError.errorString());
                qDebug() << "BackendAIProvider:" << errorMsg;
                
                Message message;
                message.id = m_currentMessageId;
                message.conversationId = m_currentConversationId;
                message.role = MessageRole::Assistant;
                message.text = "Error: " + errorMsg + "\nRaw response: " + QString::fromUtf8(responseData);
                message.createdAt = QDateTime::currentDateTime();
                
                m_status = Status::Error;
                m_statusMessage = errorMsg;
                emit statusChanged(m_status, m_statusMessage);
                emit messageCompleted(m_currentConversationId, m_currentMessageId, message);
            } else {
                QJsonObject response = doc.object();
                QString responseText = response["response"].toString();
                
                if (responseText.isEmpty()) {
                    // Try other common response field names
                    responseText = response["answer"].toString();
                    if (responseText.isEmpty()) {
                        responseText = response["text"].toString();
                    }
                    if (responseText.isEmpty()) {
                        responseText = response["content"].toString();
                    }
                    if (responseText.isEmpty()) {
                        responseText = QString("Server returned JSON but no recognizable response field. Full response: %1")
                                         .arg(QString::fromUtf8(responseData));
                    }
                }
                
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
            }
        }
    } else {
        // Handle HTTP errors or network errors
        QString errorMsg;
        if (httpStatus != 200 && httpStatus != 0) {
            errorMsg = QString("HTTP %1 error").arg(httpStatus);
            if (!responseData.isEmpty()) {
                errorMsg += QString(": %1").arg(QString::fromUtf8(responseData));
            }
        } else {
            errorMsg = reply->errorString();
            if (!responseData.isEmpty()) {
                errorMsg += QString(" (Response: %1)").arg(QString::fromUtf8(responseData));
            }
        }
        
        qDebug() << "BackendAIProvider: Error occurred:" << errorMsg;
        
        Message message;
        message.id = m_currentMessageId;
        message.conversationId = m_currentConversationId;
        message.role = MessageRole::Assistant;
        message.text = "Error: " + errorMsg;
        message.createdAt = QDateTime::currentDateTime();
        
        m_status = Status::Error;
        m_statusMessage = errorMsg;
        emit statusChanged(m_status, m_statusMessage);
        emit messageCompleted(m_currentConversationId, m_currentMessageId, message);
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
