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

namespace GadAI {

EchoProvider::EchoProvider(QObject *parent)
    : AIProvider(parent)
    , m_responseTimer(new QTimer(this))
    , m_typingTimer(new QTimer(this))
    , m_typingPosition(0)
    , m_isGenerating(false)
    , m_responseDelay(1000)
    , m_typingSpeed(50)
    , m_enableTyping(true)
    , m_enableMarkdown(true)
{
    m_currentModel = defaultModel();
    
    // Setup timers
    m_responseTimer->setSingleShot(true);
    QObject::connect(m_responseTimer, &QTimer::timeout, this, &EchoProvider::onResponseTimer);
    QObject::connect(m_typingTimer, &QTimer::timeout, this, &EchoProvider::onTypingTimer);
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
    
    // Apply configuration
    m_responseDelay = config.value("responseDelay").toInt(1000);
    m_typingSpeed = config.value("typingSpeed").toInt(50);
    m_enableTyping = config.value("enableTyping").toBool(true);
    m_enableMarkdown = config.value("enableMarkdown").toBool(true);
    
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
    return new EchoProviderConfigWidget(parent);
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
    
    // Simulate response delay
    m_responseTimer->start(m_responseDelay);
    
    // Store the message and attachments for processing
    m_pendingMessage = message;
    m_pendingAttachments = attachments;
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
    m_typingTimer->stop();
    
    m_isGenerating = false;
    m_currentConversationId.clear();
    m_currentMessageId.clear();
    m_currentResponse.clear();
    m_typingPosition = 0;
    
    qDebug() << "Echo provider: Generation stopped for conversation" << conversationId;
}

void EchoProvider::setModel(const QString &model)
{
    if (supportedModels().contains(model)) {
        m_currentModel = model;
        emit modelChanged(model);
        
        // Adjust behavior based on model
        if (model == "echo-fast") {
            m_responseDelay = 500;
            m_typingSpeed = 30;
        } else if (model == "echo-creative") {
            m_responseDelay = 1500;
            m_typingSpeed = 80;
        } else if (model == "echo-analytical") {
            m_responseDelay = 2000;
            m_typingSpeed = 40;
        } else {
            m_responseDelay = 1000;
            m_typingSpeed = 50;
        }
    }
}

void EchoProvider::onResponseTimer()
{
    if (!m_isGenerating) {
        return;
    }
    
    // Generate echo response
    m_currentResponse = generateEchoResponse(m_pendingMessage, m_pendingAttachments);
    
    if (m_enableTyping) {
        startTypingSimulation(m_currentConversationId, m_currentMessageId, m_currentResponse);
    } else {
        // Send complete response immediately
    Message responseMessage;
    responseMessage.id = m_currentMessageId;
    responseMessage.conversationId = m_currentConversationId;
    responseMessage.role = MessageRole::Assistant;
    responseMessage.text = m_currentResponse;
    responseMessage.createdAt = QDateTime::currentDateTime();
    responseMessage.metadata.insert("model", m_currentModel);
    emit messageCompleted(m_currentConversationId, m_currentMessageId, responseMessage);
        
        m_isGenerating = false;
        m_currentConversationId.clear();
        m_currentMessageId.clear();
        m_currentResponse.clear();
    }
}

void EchoProvider::onTypingTimer()
{
    if (!m_isGenerating || m_currentResponse.isEmpty()) {
        return;
    }
    
    // Send next character(s)
    int charsToSend = 1;
    
    // Send words for faster typing on spaces
    if (m_typingPosition < m_currentResponse.length() && 
        m_currentResponse[m_typingPosition].isSpace()) {
        charsToSend = 1;
    }
    
    QString chunk = m_currentResponse.mid(m_typingPosition, charsToSend);
    m_typingPosition += charsToSend;
    
    emit messageChunk(m_currentConversationId, m_currentMessageId, chunk);
    
    // Check if we've sent the entire response
    if (m_typingPosition >= m_currentResponse.length()) {
        // Complete the message
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
        m_typingPosition = 0;
        m_typingTimer->stop();
    }
}

QString EchoProvider::generateEchoResponse(
    const QString &userMessage,
    const QList<Attachment> &attachments) const
{
    QStringList responses;
    
    // Add model-specific personality
    QString personality;
    if (m_currentModel == "echo-creative") {
        personality = "I'm feeling quite creative today! ";
    } else if (m_currentModel == "echo-analytical") {
        personality = "Let me analyze that thoughtfully: ";
    } else if (m_currentModel == "echo-fast") {
        personality = "Quick response: ";
    }
    
    if (!personality.isEmpty()) {
        responses << personality;
    }
    
    // Echo the message with various formats
    if (m_enableMarkdown) {
        responses << QString("You said: **\"%1\"**").arg(userMessage);
    } else {
        responses << QString("You said: \"%1\"").arg(userMessage);
    }
    
    // Handle attachments
    if (!attachments.isEmpty()) {
        responses << QString("\nI see you've attached %1 file(s):").arg(attachments.size());
        for (const auto &attachment : attachments) {
            QString typeStr;
            switch (attachment.type) {
            case AttachmentType::Image:
                typeStr = "🖼️ Image";
                break;
            case AttachmentType::Audio:
                typeStr = "🎵 Audio";
                break;
            case AttachmentType::Text:
                typeStr = "📄 Text";
                break;
            default:
                typeStr = "📎 File";
                break;
            }
            responses << QString("- %1: %2").arg(typeStr, attachment.fileName);
        }
    }
    
    // Add some randomized responses
    QStringList echoVariations = {
        "That's interesting! Here's what I understood from your message:",
        "I hear you loud and clear! You mentioned:",
        "Thanks for sharing that with me. To confirm, you said:",
        "I'm processing your input. You communicated:",
        "Your message has been received. The content was:"
    };
    
    if (m_enableMarkdown) {
        echoVariations.append({
            "*Thoughtfully reflecting on your words...*",
            "**Processing complete!** Your message contained:",
            "~~Thinking~~ Done thinking! You expressed:",
            "> Your message resonates with me. You said:"
        });
    }
    
    int randomIndex = QRandomGenerator::global()->bounded(echoVariations.size());
    responses << "\n" + echoVariations[randomIndex];
    responses << QString("\"%1\"").arg(userMessage);
    
    // Add message statistics
    responses << QString("\n📊 **Message Statistics:**");
    responses << QString("- Characters: %1").arg(userMessage.length());
    responses << QString("- Words: %1").arg(userMessage.split(' ', Qt::SkipEmptyParts).size());
    responses << QString("- Attachments: %1").arg(attachments.size());
    responses << QString("- Model: %1").arg(m_currentModel);
    responses << QString("- Timestamp: %1").arg(QDateTime::currentDateTime().toString());
    
    return responses.join("\n");
}

void EchoProvider::startTypingSimulation(
    const QString &conversationId,
    const QString &messageId,
    const QString &fullResponse)
{
    m_currentConversationId = conversationId;
    m_currentMessageId = messageId;
    m_currentResponse = fullResponse;
    m_typingPosition = 0;
    
    m_typingTimer->start(m_typingSpeed);
}

// EchoProviderConfigWidget implementation
EchoProviderConfigWidget::EchoProviderConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    
    // Timing settings
    auto *timingGroup = new QGroupBox("Timing Settings");
    auto *timingLayout = new QFormLayout(timingGroup);
    
    m_responseDelaySpinBox = new QSpinBox();
    m_responseDelaySpinBox->setRange(100, 10000);
    m_responseDelaySpinBox->setSuffix(" ms");
    m_responseDelaySpinBox->setValue(1000);
    timingLayout->addRow("Response Delay:", m_responseDelaySpinBox);
    
    m_typingSpeedSpinBox = new QSpinBox();
    m_typingSpeedSpinBox->setRange(10, 500);
    m_typingSpeedSpinBox->setSuffix(" ms");
    m_typingSpeedSpinBox->setValue(50);
    timingLayout->addRow("Typing Speed:", m_typingSpeedSpinBox);
    
    layout->addWidget(timingGroup);
    
    // Behavior settings
    auto *behaviorGroup = new QGroupBox("Behavior Settings");
    auto *behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    m_enableTypingCheckBox = new QCheckBox("Enable typing simulation");
    m_enableTypingCheckBox->setChecked(true);
    behaviorLayout->addWidget(m_enableTypingCheckBox);
    
    m_enableMarkdownCheckBox = new QCheckBox("Enable markdown formatting");
    m_enableMarkdownCheckBox->setChecked(true);
    behaviorLayout->addWidget(m_enableMarkdownCheckBox);
    
    layout->addWidget(behaviorGroup);
    
    // Info
    auto *infoLabel = new QLabel(
        "The Echo Provider is a demonstration provider that echoes your messages "
        "with simulated AI behavior. It's useful for testing the application "
        "without requiring an external AI service."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #6B7280; font-size: 12px; padding: 8px;");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
}

QJsonObject EchoProviderConfigWidget::getConfig() const
{
    return QJsonObject{
        {"responseDelay", m_responseDelaySpinBox->value()},
        {"typingSpeed", m_typingSpeedSpinBox->value()},
        {"enableTyping", m_enableTypingCheckBox->isChecked()},
        {"enableMarkdown", m_enableMarkdownCheckBox->isChecked()}
    };
}

void EchoProviderConfigWidget::setConfig(const QJsonObject &config)
{
    m_responseDelaySpinBox->setValue(config.value("responseDelay").toInt(1000));
    m_typingSpeedSpinBox->setValue(config.value("typingSpeed").toInt(50));
    m_enableTypingCheckBox->setChecked(config.value("enableTyping").toBool(true));
    m_enableMarkdownCheckBox->setChecked(config.value("enableMarkdown").toBool(true));
}

} // namespace GadAI

// MOC include not required when using CMake AUTOMOC and proper Q_OBJECT usage
