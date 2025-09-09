#include "ProviderSDK.h"
#include <QDebug>

namespace GadAI {

// AIProvider implementation
AIProvider::AIProvider(QObject *parent)
    : QObject(parent)
{
}

// ProviderRegistry implementation
ProviderRegistry::ProviderRegistry(QObject *parent)
    : QObject(parent)
{
}

void ProviderRegistry::registerProvider(const QString &id, std::function<AIProvider*()> factory)
{
    // Create a temporary instance to get provider info
    std::unique_ptr<AIProvider> tempProvider(factory());
    
    ProviderInfo info;
    info.factory = factory;
    info.name = tempProvider->name();
    info.description = tempProvider->description();
    info.icon = tempProvider->icon();
    
    m_providers[id] = info;
    
    qDebug() << "Registered provider:" << id << info.name;
}

void ProviderRegistry::unregisterProvider(const QString &id)
{
    m_providers.remove(id);
    qDebug() << "Unregistered provider:" << id;
}

AIProvider *ProviderRegistry::createProvider(const QString &id, QObject *parent)
{
    auto it = m_providers.find(id);
    if (it != m_providers.end()) {
        AIProvider *provider = it->factory();
        if (provider && parent) {
            provider->setParent(parent);
        }
        return provider;
    }
    
    qWarning() << "Unknown provider:" << id;
    return nullptr;
}

QStringList ProviderRegistry::availableProviders() const
{
    return m_providers.keys();
}

QString ProviderRegistry::providerName(const QString &id) const
{
    auto it = m_providers.find(id);
    return it != m_providers.end() ? it->name : QString();
}

QString ProviderRegistry::providerDescription(const QString &id) const
{
    auto it = m_providers.find(id);
    return it != m_providers.end() ? it->description : QString();
}

QIcon ProviderRegistry::providerIcon(const QString &id) const
{
    auto it = m_providers.find(id);
    return it != m_providers.end() ? it->icon : QIcon();
}

// ProviderManager implementation moved to ProviderManager.{h,cpp}

} // namespace GadAI
