#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "ProviderSDK.h"

namespace GadAI {

class ProviderManager : public QObject {
    Q_OBJECT
public:
    explicit ProviderManager(QObject *parent = nullptr);
    ~ProviderManager();

    void setActiveProvider(const QString &providerId, const QJsonObject &config = {});
    AIProvider *activeProvider() const { return m_activeProvider; }
    QString activeProviderId() const { return m_activeProviderId; }

    ProviderRegistry *registry() const { return m_registry; }

    void sendMessage(const QString &conversationId,
                     const QString &message,
                     const QList<Attachment> &attachments = {},
                     const QJsonObject &options = {});

    void regenerateResponse(const QString &conversationId, const QString &messageId);
    void stopGeneration(const QString &conversationId);

signals:
    void activeProviderChanged(const QString &providerId);
    void providerStatusChanged(AIProvider::Status status, const QString &message);

    void messageStarted(const QString &conversationId, const QString &messageId);
    void messageChunk(const QString &conversationId, const QString &messageId, const QString &content);
    void messageCompleted(const QString &conversationId, const QString &messageId, const Message &message);
    void messageFailed(const QString &conversationId, const QString &messageId, const QString &error);

private slots:
    void onProviderStatusChanged(AIProvider::Status status, const QString &message);

private:
    ProviderRegistry *m_registry;
    AIProvider *m_activeProvider;
    QString m_activeProviderId;
};

} // namespace GadAI
