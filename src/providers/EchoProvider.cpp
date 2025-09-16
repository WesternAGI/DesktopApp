#include "EchoProvider.h"
#include "core/Application.h"
#include "theme/IconRegistry.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTimer>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>
#include <QDebug>

namespace DesktopApp {

EchoProvider::EchoProvider(QObject *parent)
    : AIProvider(parent)
    , m_responseTimer(new QTimer(this))
    , m_isGenerating(false)
{
    m_currentModel = defaultModel();
    
    // Setup timers
    m_responseTimer->setSingleShot(true);
    QObject::connect(m_responseTimer, &QTimer::timeout, this, &EchoProvider::onResponseTimer);
}

QIcon EchoProvider::icon() const
{
    auto *app = Application::instance();
    if (app && app->iconRegistry()) {
        return app->iconRegistry()->icon("message-circle");
    }
    return QIcon();
}

AIProvider::Capabilities EchoProvider::capabilities() const
{
    return Capability::TextGeneration | Capability::Streaming;
}

QStringList EchoProvider::supportedModels() const
{
    return {
        "echo-v1",
        "echo-fast",
        "echo-creative",
        "echo-analytical"
    };
}

void EchoProvider::connect(const QJsonObject &config)
{
    m_config = config;
    
    // Simulate connection delay
    m_status = Status::Connecting;
    m_statusMessage = "Connecting to Echo Provider...";
    emit statusChanged(m_status, m_statusMessage);
    
    QTimer::singleShot(500, this, [this]() {
        m_status = Status::Connected;
        m_statusMessage = "Connected to Echo Provider";
        emit statusChanged(m_status, m_statusMessage);
        emit connected();
    });
}

void EchoProvider::disconnect()
{
    // Stop any ongoing generation
    if (m_isGenerating) {
        stopGeneration(m_currentConversationId);
    }
    
    m_status = Status::Disconnected;
    m_statusMessage = "Disconnected";
    emit statusChanged(m_status, m_statusMessage);
    emit disconnected();
}

QJsonObject EchoProvider::defaultConfig() const
{
    return QJsonObject{
        {"responseDelay", 1000},
        {"typingSpeed", 50},
        {"enableTyping", true},
        {"enableMarkdown", true}
    };
}

bool EchoProvider::validateConfig(const QJsonObject &config) const
{
    // Basic validation for echo provider
    return config.contains("responseDelay") &&
           config.contains("typingSpeed") &&
           config.contains("enableTyping") &&
           config.contains("enableMarkdown");
}

QWidget *EchoProvider::createConfigWidget(QWidget *parent)
{
    return nullptr;
}

void EchoProvider::sendMessage(
    const QString &conversationId,
    const QString &message,
    const QList<Attachment> &attachments,
    const QJsonObject &options)
{
    Q_UNUSED(options)
    
    if (m_status != Status::Connected) {
        emit messageFailed(conversationId, "", "Provider not connected");
        return;
    }
    
    if (m_isGenerating) {
        emit messageFailed(conversationId, "", "Provider is busy");
        return;
    }
    
    m_isGenerating = true;
    m_currentConversationId = conversationId;
    m_currentMessageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    emit messageStarted(conversationId, m_currentMessageId);
    
    // Store the message and attachments for processing
    m_pendingMessage = message;
    m_pendingAttachments = attachments;
    
    // Trigger immediate response (no delay for strict echo behavior)
    m_responseTimer->start(1); // 1ms minimal delay for async processing
}

void EchoProvider::regenerateResponse(const QString &conversationId, const QString &messageId)
{
    Q_UNUSED(messageId)
    
    if (m_status != Status::Connected) {
        emit messageFailed(conversationId, "", "Provider not connected");
        return;
    }
    
    // For echo provider, just regenerate the last response
    sendMessage(conversationId, m_pendingMessage, m_pendingAttachments);
}

void EchoProvider::stopGeneration(const QString &conversationId)
{
    if (!m_isGenerating || m_currentConversationId != conversationId) {
        return;
    }
    
    m_responseTimer->stop();
    
    m_isGenerating = false;
    m_currentConversationId.clear();
    m_currentMessageId.clear();
    m_currentResponse.clear();
    
    qDebug() << "Echo provider: Generation stopped for conversation" << conversationId;
}

void EchoProvider::setModel(const QString &model)
{
    if (supportedModels().contains(model)) {
        m_currentModel = model;
        emit modelChanged(model);
    }
}

void EchoProvider::onResponseTimer()
{
    if (!m_isGenerating) {
        return;
    }
    
    // Generate strict echo response (exactly what user typed)
    m_currentResponse = generateEchoResponse(m_pendingMessage, m_pendingAttachments);
    
    // Send complete response immediately without typing simulation
    Message responseMessage;
    responseMessage.id = m_currentMessageId;
    responseMessage.conversationId = m_currentConversationId;
    responseMessage.role = MessageRole::Assistant;
    responseMessage.text = m_currentResponse;
    responseMessage.createdAt = QDateTime::currentDateTime();
    responseMessage.metadata.insert("model", m_currentModel);
    emit messageCompleted(m_currentConversationId, m_currentMessageId, responseMessage);
    
    // Reset state
    m_isGenerating = false;
    m_currentConversationId.clear();
    m_currentMessageId.clear();
    m_currentResponse.clear();
}

QString EchoProvider::generateEchoResponse(
    const QString &userMessage,
    const QList<DesktopApp::Attachment> &attachments) const
{
    Q_UNUSED(attachments)
    
    // Strict echo behavior: return exactly what the user typed
    return userMessage;
}

} // namespace DesktopApp

// MOC include not required when using CMake AUTOMOC and proper Q_OBJECT usage
