#include "Application.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"
#include "data/JsonStore.h"
#include "services/SettingsStore.h"
#include "services/FileVault.h"
#include "services/SearchEngine.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include "providers/ProviderSDK.h"
#include "providers/ProviderManager.h"
#include "providers/EchoProvider.h"
#include "services/AudioRecorder.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

namespace DesktopApp {

Application* Application::s_instance = nullptr;

Application::Application(QObject *parent)
    : QObject(parent)
{
    s_instance = this;
}

Application::~Application()
{
    s_instance = nullptr;
}

bool Application::initialize()
{
    qDebug() << "Initializing DesktopApp Application...";

    // Initialize directories first
    if (!initializeDirectories()) {
        qCritical() << "Failed to initialize application directories";
        return false;
    }

    // Initialize all services
    initializeServices();

    qDebug() << "DesktopApp Application initialized successfully";
    return true;
}

Application* Application::instance()
{
    return s_instance;
}

bool Application::initializeDirectories()
{
    // Set up application data directories
    m_appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    // Create directories if they don't exist
    QStringList dirs = {m_appDataDir, m_cacheDir, m_configDir};
    for (const QString& dir : dirs) {
        QDir qdir(dir);
        if (!qdir.exists() && !qdir.mkpath(dir)) {
            qCritical() << "Failed to create directory:" << dir;
            return false;
        }
    }

    qDebug() << "Application directories initialized:";
    qDebug() << "  Data:" << m_appDataDir;
    qDebug() << "  Cache:" << m_cacheDir;
    qDebug() << "  Config:" << m_configDir;

    return true;
}

void Application::initializeServices()
{
    // Initialize theme manager first (other services may depend on it)
    m_themeManager = std::make_unique<ThemeManager>(this);
    connect(m_themeManager.get(), &ThemeManager::themeChanged,
            this, &Application::onThemeChanged);

    // Initialize icon registry
    m_iconRegistry = std::make_unique<IconRegistry>(this);

    // Initialize settings store
    m_settingsStore = std::make_unique<SettingsStore>(m_configDir, this);

    // Initialize file vault
    m_fileVault = std::make_unique<FileVault>(m_appDataDir + "/attachments", this);

    // Initialize conversation store with JSON storage
    m_conversationStore = std::make_unique<JsonStore>(this);
    if (!m_conversationStore->initialize(m_appDataDir)) {
        qCritical() << "Failed to initialize JSON store";
        return;
    }

    // Initialize search engine (depends on conversation store)
    m_searchEngine = std::make_unique<SearchEngine>(m_conversationStore.get(), this);

    // Initialize provider manager and register built-in providers
    m_providerManager = std::make_unique<ProviderManager>(this);
    m_providerManager->registry()->registerProvider("echo", [](){ return new EchoProvider(); });
    // Determine config (persisted or default)
    QJsonObject echoCfg;
    QVariant stored = m_settingsStore->value("providers/echo/config");
    if (stored.isValid()) {
        int id = stored.metaType().id();
        if (id == QMetaType::QVariantMap) {
            echoCfg = QJsonObject::fromVariantMap(stored.toMap());
        } else if (id == QMetaType::QString) {
            QJsonParseError pe; auto doc = QJsonDocument::fromJson(stored.toString().toUtf8(), &pe);
            if (pe.error == QJsonParseError::NoError && doc.isObject()) echoCfg = doc.object();
        }
    }
    m_providerManager->setActiveProvider("echo");
    if (auto *prov = m_providerManager->activeProvider()) {
        if (echoCfg.isEmpty()) echoCfg = prov->defaultConfig();
        prov->connect(echoCfg);
        // Persist on future status/config changes
        connect(prov, &AIProvider::statusChanged, this, [this, prov]() {
            m_settingsStore->setValue("providers/echo/config", prov->currentConfig());
        });
    }

    // Load initial theme
    QString savedTheme = m_settingsStore->value("ui/theme", "light").toString();
    m_themeManager->setTheme(savedTheme);

    // Initialize audio recorder
    m_audioRecorder = std::make_unique<AudioRecorder>(this);

    qDebug() << "All services initialized successfully";
}

void Application::onThemeChanged()
{
    // Save theme preference
    m_settingsStore->setValue("ui/theme", m_themeManager->currentTheme());
    emit themeChanged();
}

} // namespace DesktopApp
