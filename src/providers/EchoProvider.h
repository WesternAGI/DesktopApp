#pragma once

#include "ProviderSDK.h"
#include <QTimer>
#include <QJsonObject>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>

namespace DesktopApp {

/**
 * @brief Echo provider for offline demonstration
 * 
 * A simple AI provider that echoes user messages with simulated delays
 * and typing effects. Useful for testing and demonstration purposes.
 */
class EchoProvider : public AIProvider
{
    Q_OBJECT

public:
    explicit EchoProvider(QObject *parent = nullptr);

    // Provider identification
    QString id() const override { return "echo"; }
    QString name() const override { return "Echo Provider"; }
    QString description() const override { return "Local echo provider for testing and demonstration"; }
    QString version() const override { return "1.0.0"; }
    QIcon icon() const override;

    // Provider capabilities
    Capabilities capabilities() const override;
    QStringList supportedModels() const override;
    QString defaultModel() const override { return "echo-v1"; }

    // Connection management
    void connect(const QJsonObject &config) override;
    void disconnect() override;
    Status status() const override { return m_status; }
    QString statusMessage() const override { return m_statusMessage; }

    // Configuration
    QJsonObject defaultConfig() const override;
    bool validateConfig(const QJsonObject &config) const override;
    QWidget *createConfigWidget(QWidget *parent = nullptr) override;

    // Message processing
    void sendMessage(
        const QString &conversationId,
        const QString &message,
        const QList<Attachment> &attachments = {},
        const QJsonObject &options = {}
    ) override;

    void regenerateResponse(
        const QString &conversationId,
        const QString &messageId
    ) override;

    void stopGeneration(const QString &conversationId) override;

    // Model management
    void setModel(const QString &model) override;
    QString currentModel() const override { return m_currentModel; }

private slots:
    void onResponseTimer();

private:
    void simulateResponse(
        const QString &conversationId,
        const QString &userMessage,
        const QList<Attachment> &attachments
    );
    
    QString generateEchoResponse(
        const QString &userMessage,
        const QList<Attachment> &attachments
    ) const;

    // Response simulation
    QTimer *m_responseTimer;
    
    // Current generation state
    QString m_currentConversationId;
    QString m_currentMessageId;
    QString m_currentResponse;
    bool m_isGenerating;

    // Pending message storage
    QString m_pendingMessage;
    QList<Attachment> m_pendingAttachments;
};

/**
 * @brief Configuration widget for EchoProvider
 */
class EchoProviderConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EchoProviderConfigWidget(QWidget *parent = nullptr);
    
    QJsonObject getConfig() const;
    void setConfig(const QJsonObject &config);

private:
    class QSpinBox *m_responseDelaySpinBox;
    class QSpinBox *m_typingSpeedSpinBox;
    class QCheckBox *m_enableTypingCheckBox;
    class QCheckBox *m_enableMarkdownCheckBox;
};

} // namespace DesktopApp
