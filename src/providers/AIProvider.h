#pragma once

#include "providers/ProviderSDK.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

namespace DesktopApp {

/**
 * @brief AI Provider that connects to the backend API
 */
class BackendAIProvider : public AIProvider
{
    Q_OBJECT

public:
    explicit BackendAIProvider(QObject *parent = nullptr);
    ~BackendAIProvider() override = default;

    // Provider identification
    QString id() const override { return "backend_ai"; }
    QString name() const override { return "Backend AI"; }
    QString description() const override { return "Connect to AI backend service"; }
    QString version() const override { return "1.0.0"; }
    QIcon icon() const override;
    
    // Provider capabilities
    Capabilities capabilities() const override;
    QStringList supportedModels() const override;
    QString defaultModel() const override;
    
    // Status
    Status status() const override { return m_status; }
    QString statusMessage() const override;
    
    // Configuration
    QJsonObject defaultConfig() const override;
    bool validateConfig(const QJsonObject &config) const override;
    QWidget* createConfigWidget(QWidget *parent = nullptr) override;
    
    void connect(const QJsonObject &config) override;
    void disconnect() override;

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

    void setModel(const QString &model) override;
    QString currentModel() const override;

    void setAuthToken(const QString &token);

private slots:
    void onReplyFinished();
    void onError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_authToken;
    QString m_baseUrl;
    QNetworkReply *m_currentReply;
    QString m_currentConversationId;
    QString m_currentMessageId;
    QString m_currentModel;
    Status m_status;
    QString m_statusMessage;
};

} // namespace DesktopApp
