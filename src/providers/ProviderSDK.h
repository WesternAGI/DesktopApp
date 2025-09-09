#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QWidget>
#include "data/Models.h"

namespace GadAI {

/**
 * @brief Abstract base class for AI providers
 * 
 * Defines the interface that all AI providers must implement to integrate
 * with the GadAI application. This allows for pluggable AI backends.
 */
class AIProvider : public QObject
{
    Q_OBJECT

public:
    enum class Status {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    enum class Capability {
        TextGeneration = 1 << 0,
        ImageGeneration = 1 << 1,
        ImageAnalysis = 1 << 2,
        AudioTranscription = 1 << 3,
        AudioGeneration = 1 << 4,
        FunctionCalling = 1 << 5,
        Streaming = 1 << 6
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    explicit AIProvider(QObject *parent = nullptr);
    virtual ~AIProvider() = default;

    // Provider identification
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString version() const = 0;
    virtual QIcon icon() const = 0;

    // Provider capabilities
    virtual Capabilities capabilities() const = 0;
    virtual QStringList supportedModels() const = 0;
    virtual QString defaultModel() const = 0;

    // Connection management
    virtual void connect(const QJsonObject &config) = 0;
    virtual void disconnect() = 0;
    virtual Status status() const = 0;
    virtual QString statusMessage() const = 0;

    // Configuration
    virtual QJsonObject defaultConfig() const = 0;
    virtual bool validateConfig(const QJsonObject &config) const = 0;
    virtual QWidget *createConfigWidget(QWidget *parent = nullptr) = 0;

    // Message processing
    virtual void sendMessage(
        const QString &conversationId,
        const QString &message,
        const QList<Attachment> &attachments = {},
        const QJsonObject &options = {}
    ) = 0;

    virtual void regenerateResponse(
        const QString &conversationId,
        const QString &messageId
    ) = 0;

    virtual void stopGeneration(const QString &conversationId) = 0;

    // Model management
    virtual void setModel(const QString &model) = 0;
    virtual QString currentModel() const = 0;

signals:
    // Connection events
    void statusChanged(Status status, const QString &message = QString());
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

    // Message events
    void messageStarted(const QString &conversationId, const QString &messageId);
    void messageChunk(const QString &conversationId, const QString &messageId, const QString &content);
    void messageCompleted(const QString &conversationId, const QString &messageId, const Message &message);
    void messageFailed(const QString &conversationId, const QString &messageId, const QString &error);

    // Model events
    void modelChanged(const QString &model);
    void modelsUpdated(const QStringList &models);

protected:
    Status m_status = Status::Disconnected;
    QString m_statusMessage;
    QString m_currentModel;
    QJsonObject m_config;

public:
    const QJsonObject &currentConfig() const { return m_config; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AIProvider::Capabilities)

/**
 * @brief Provider registry and factory
 * 
 * Manages the registration and creation of AI providers.
 */
class ProviderRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ProviderRegistry(QObject *parent = nullptr);
    
    // Registration
    void registerProvider(const QString &id, std::function<AIProvider*()> factory);
    void unregisterProvider(const QString &id);
    
    // Provider creation
    AIProvider *createProvider(const QString &id, QObject *parent = nullptr);
    QStringList availableProviders() const;
    
    // Provider information
    QString providerName(const QString &id) const;
    QString providerDescription(const QString &id) const;
    QIcon providerIcon(const QString &id) const;

private:
    struct ProviderInfo {
        std::function<AIProvider*()> factory;
        QString name;
        QString description;
        QIcon icon;
    };
    
    QHash<QString, ProviderInfo> m_providers;
};

/**
 * @brief Provider manager
 * 
 * Manages active provider instances and routing of messages.
 */
// ProviderManager now separated into its own header (ProviderManager.h)

} // namespace GadAI
