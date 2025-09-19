#pragma once

#include "data/JsonStore.h"
#include <memory>

namespace DesktopApp {

class ThemeManager;
class IconRegistry;
class JsonStore;
class SettingsStore;
class FileVault;
class SearchEngine;
class ProviderManager;
class AudioRecorder;
class AuthenticationService;

/**
 * @brief Core application class that manages global services and initialization
 */
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

    /**
     * @brief Initialize the application and all services
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Get the global application instance
     */
    static Application* instance();

    // Service getters
    ThemeManager* themeManager() const { return m_themeManager.get(); }
    IconRegistry* iconRegistry() const { return m_iconRegistry.get(); }
    JsonStore *conversationStore() const { return m_conversationStore.get(); }
    SettingsStore* settingsStore() const { return m_settingsStore.get(); }
    FileVault* fileVault() const { return m_fileVault.get(); }
    SearchEngine* searchEngine() const { return m_searchEngine.get(); }
    ProviderManager* providerManager() const { return m_providerManager.get(); }
    AudioRecorder* audioRecorder() const { return m_audioRecorder.get(); }
    AuthenticationService* authenticationService() const { return m_authenticationService.get(); }

    // Directories
    const QString &appDataDir() const { return m_appDataDir; }
    const QString &cacheDir() const { return m_cacheDir; }
    const QString &configDir() const { return m_configDir; }

signals:
    void themeChanged();
    void settingsChanged();

private slots:
    void onThemeChanged();

private:
    bool initializeDirectories();
    void initializeServices();

    static Application* s_instance;

    // Core services
    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<IconRegistry> m_iconRegistry;
    std::unique_ptr<JsonStore> m_conversationStore;
    std::unique_ptr<SettingsStore> m_settingsStore;
    std::unique_ptr<FileVault> m_fileVault;
    std::unique_ptr<SearchEngine> m_searchEngine;
    std::unique_ptr<ProviderManager> m_providerManager;
    std::unique_ptr<AudioRecorder> m_audioRecorder;
    std::unique_ptr<AuthenticationService> m_authenticationService;

    // Application directories
    QString m_appDataDir;
    QString m_cacheDir;
    QString m_configDir;
};

} // namespace DesktopApp
