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

    // AbstractProvider interface
    QString id() const override { return "backend_ai"; }
    QString name() const override { return "Backend AI"; }
    QString description() const override { return "Connect to AI backend service"; }
    QString version() const override { return "1.0.0"; }
    QIcon icon() const override;
    
    bool isAvailable() const override;
    Status status() const override { return m_status; }
    QWidget* configWidget(QWidget *parent = nullptr) override;
    bool validateConfig(const QJsonObject &config) const override;
    
    void connect(const QJsonObject &config) override;
    void disconnect() override;

    void sendMessage(const QString &conversationId, 
                    const QString &messageId, 
                    const QString &text,
                    const QList<Attachment> &attachments = {},
                    const QJsonObject &options = {}) override;

    void setAuthToken(const QString &token);

private slots:
    void onReplyFinished();
    void onError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_authToken;
    QString m_baseUrl;
    Status m_status;
    QNetworkReply *m_currentReply;
    QString m_currentConversationId;
    QString m_currentMessageId;
};

} // namespace DesktopApp
