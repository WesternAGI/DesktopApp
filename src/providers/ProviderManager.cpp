#include "ProviderManager.h"

namespace GadAI {

ProviderManager::ProviderManager(QObject *parent)
    : QObject(parent)
    , m_registry(new ProviderRegistry(this))
    , m_activeProvider(nullptr)
{
}

ProviderManager::~ProviderManager()
{
    if (m_activeProvider) {
        m_activeProvider->disconnect();
        m_activeProvider->deleteLater();
    }
}

void ProviderManager::setActiveProvider(const QString &providerId, const QJsonObject &config)
{
    if (m_activeProviderId == providerId && m_activeProvider) {
        // Already active; optionally update config
        if (!config.isEmpty()) {
            m_activeProvider->connect(config); // Reconnect with new config
        }
        return;
    }

    if (m_activeProvider) {
        disconnect(m_activeProvider, nullptr, this, nullptr);
        m_activeProvider->disconnect();
        m_activeProvider->deleteLater();
        m_activeProvider = nullptr;
    }

    m_activeProvider = m_registry->createProvider(providerId, this);
    if (!m_activeProvider) {
        return;
    }

    m_activeProviderId = providerId;

    connect(m_activeProvider, &AIProvider::statusChanged,
            this, &ProviderManager::onProviderStatusChanged);
    connect(m_activeProvider, &AIProvider::messageStarted,
            this, &ProviderManager::messageStarted);
    connect(m_activeProvider, &AIProvider::messageChunk,
            this, &ProviderManager::messageChunk);
    connect(m_activeProvider, &AIProvider::messageCompleted,
            this, &ProviderManager::messageCompleted);
    connect(m_activeProvider, &AIProvider::messageFailed,
            this, &ProviderManager::messageFailed);

    if (!config.isEmpty()) {
        m_activeProvider->connect(config);
    }

    emit activeProviderChanged(providerId);
}

void ProviderManager::sendMessage(const QString &conversationId,
                                  const QString &message,
                                  const QList<Attachment> &attachments,
                                  const QJsonObject &options)
{
    if (!m_activeProvider) return;
    m_activeProvider->sendMessage(conversationId, message, attachments, options);
}

void ProviderManager::regenerateResponse(const QString &conversationId, const QString &messageId)
{
    if (!m_activeProvider) return;
    m_activeProvider->regenerateResponse(conversationId, messageId);
}

void ProviderManager::stopGeneration(const QString &conversationId)
{
    if (!m_activeProvider) return;
    m_activeProvider->stopGeneration(conversationId);
}

void ProviderManager::onProviderStatusChanged(AIProvider::Status status, const QString &message)
{
    emit providerStatusChanged(status, message);
}

} // namespace GadAI
