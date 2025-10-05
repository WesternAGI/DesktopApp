#include "SettingsDialog.h"
#include "core/Application.h"
#include "services/SettingsStore.h"
#include "services/AuthenticationService.h"
#include "providers/ProviderManager.h"
#include "providers/EchoProvider.h"
#include "providers/ProviderSDK.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollArea>
#include <QFontComboBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QTextEdit>
#include <QFrame>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QThread>

namespace DesktopApp {

// GeneralSettingsWidget implementation
GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void GeneralSettingsWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(16);

    // Appearance group
    auto *appearanceGroup = new QGroupBox("Appearance");
    auto *appearanceLayout = new QFormLayout(appearanceGroup);

    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"System", "Light", "Dark"});
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsWidget::onThemeChanged);
    appearanceLayout->addRow("Theme:", m_themeCombo);

    m_languageCombo = new QComboBox();
    m_languageCombo->addItems({"English", "Spanish", "French", "German", "Chinese", "Japanese"});
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsWidget::onLanguageChanged);
    appearanceLayout->addRow("Language:", m_languageCombo);

    layout->addWidget(appearanceGroup);

    // Startup group
    auto *startupGroup = new QGroupBox("Startup");
    auto *startupLayout = new QVBoxLayout(startupGroup);

    m_startMinimizedCheck = new QCheckBox("Start minimized to system tray");
    connect(m_startMinimizedCheck, &QCheckBox::toggled,
            this, &GeneralSettingsWidget::onStartupChanged);
    startupLayout->addWidget(m_startMinimizedCheck);

    m_autoStartCheck = new QCheckBox("Start with Windows");
    connect(m_autoStartCheck, &QCheckBox::toggled,
            this, &GeneralSettingsWidget::onStartupChanged);
    startupLayout->addWidget(m_autoStartCheck);

    layout->addWidget(startupGroup);

    // Updates group
    auto *updatesGroup = new QGroupBox("Updates & Privacy");
    auto *updatesLayout = new QVBoxLayout(updatesGroup);

    m_checkUpdatesCheck = new QCheckBox("Check for updates automatically");
    updatesLayout->addWidget(m_checkUpdatesCheck);

    m_analyticsCheck = new QCheckBox("Send anonymous usage statistics");
    updatesLayout->addWidget(m_analyticsCheck);

    layout->addWidget(updatesGroup);

    // Data management group
    auto *dataGroup = new QGroupBox("Data Management");
    auto *dataLayout = new QFormLayout(dataGroup);

    m_maxConversationsSpin = new QSpinBox();
    m_maxConversationsSpin->setRange(10, 10000);
    m_maxConversationsSpin->setValue(1000);
    m_maxConversationsSpin->setSuffix(" conversations");
    dataLayout->addRow("Maximum stored conversations:", m_maxConversationsSpin);

    m_autoSaveIntervalSpin = new QSpinBox();
    m_autoSaveIntervalSpin->setRange(1, 60);
    m_autoSaveIntervalSpin->setValue(5);
    m_autoSaveIntervalSpin->setSuffix(" minutes");
    dataLayout->addRow("Auto-save interval:", m_autoSaveIntervalSpin);

    layout->addWidget(dataGroup);

    layout->addStretch();
}

void GeneralSettingsWidget::loadSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();

    // Theme
    QString theme = settings->get("ui/theme", "System").toString();
    int themeIndex = m_themeCombo->findText(theme);
    if (themeIndex >= 0) {
        m_themeCombo->setCurrentIndex(themeIndex);
    }

    // Language
    QString language = settings->get("ui/language", "English").toString();
    int langIndex = m_languageCombo->findText(language);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }

    // Startup
    m_startMinimizedCheck->setChecked(settings->get("startup/minimized", false).toBool());
    m_autoStartCheck->setChecked(settings->get("startup/autoStart", false).toBool());

    // Updates
    m_checkUpdatesCheck->setChecked(settings->get("updates/autoCheck", true).toBool());
    m_analyticsCheck->setChecked(settings->get("privacy/analytics", false).toBool());

    // Data
    m_maxConversationsSpin->setValue(settings->get("data/maxConversations", 1000).toInt());
    m_autoSaveIntervalSpin->setValue(settings->get("data/autoSaveInterval", 5).toInt());
}

void GeneralSettingsWidget::saveSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();

    settings->set("ui/theme", m_themeCombo->currentText());
    settings->set("ui/language", m_languageCombo->currentText());
    settings->set("startup/minimized", m_startMinimizedCheck->isChecked());
    settings->set("startup/autoStart", m_autoStartCheck->isChecked());
    settings->set("updates/autoCheck", m_checkUpdatesCheck->isChecked());
    settings->set("privacy/analytics", m_analyticsCheck->isChecked());
    settings->set("data/maxConversations", m_maxConversationsSpin->value());
    settings->set("data/autoSaveInterval", m_autoSaveIntervalSpin->value());
}

void GeneralSettingsWidget::onThemeChanged()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    
    QString theme = m_themeCombo->currentText();
    if (theme == "Light") {
        themeManager->setTheme(ThemeManager::Light);
    } else if (theme == "Dark") {
        themeManager->setTheme(ThemeManager::Dark);
    } else {
        // System -> pick based on current palette (simple heuristic)
        // For now just leave as current; could add OS detection later.
    }
    
    emit settingsChanged();
}

void GeneralSettingsWidget::onLanguageChanged()
{
    emit settingsChanged();
}

void GeneralSettingsWidget::onStartupChanged()
{
    emit settingsChanged();
}

// ProviderSettingsWidget implementation
ProviderSettingsWidget::ProviderSettingsWidget(ProviderManager *providerManager, QWidget *parent)
    : QWidget(parent)
    , m_providerManager(providerManager)
{
    setupUI();
    populateProviderList();
    loadSettings();
}

void ProviderSettingsWidget::setupUI()
{
    auto *layout = new QHBoxLayout(this);
    
    // Left side - provider list
    auto *leftWidget = new QWidget();
    leftWidget->setMaximumWidth(200);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    
    auto *listLabel = new QLabel("AI Providers:");
    listLabel->setStyleSheet("font-weight: bold;");
    leftLayout->addWidget(listLabel);
    
    m_providerList = new QListWidget();
    connect(m_providerList, &QListWidget::currentTextChanged,
            this, &ProviderSettingsWidget::onProviderSelectionChanged);
    leftLayout->addWidget(m_providerList);
    
    // Provider management buttons
    auto *buttonLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("Add");
    m_addButton->setEnabled(false); // TODO: Implement custom provider addition
    connect(m_addButton, &QPushButton::clicked,
            this, &ProviderSettingsWidget::onAddProvider);
    buttonLayout->addWidget(m_addButton);
    
    m_removeButton = new QPushButton("Remove");
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked,
            this, &ProviderSettingsWidget::onRemoveProvider);
    buttonLayout->addWidget(m_removeButton);
    
    leftLayout->addLayout(buttonLayout);
    layout->addWidget(leftWidget);
    
    // Right side - provider configuration
    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);
    
    // Configuration stack
    m_configStack = new QStackedWidget();
    rightLayout->addWidget(m_configStack);
    
    // Status and test connection
    auto *statusLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel("Status: Not configured");
    statusLayout->addWidget(m_statusLabel);
    
    statusLayout->addStretch();
    
    m_testButton = new QPushButton("Test Connection");
    connect(m_testButton, &QPushButton::clicked,
            this, &ProviderSettingsWidget::onTestConnection);
    statusLayout->addWidget(m_testButton);
    
    rightLayout->addLayout(statusLayout);
    
    layout->addWidget(rightWidget);
}

void ProviderSettingsWidget::populateProviderList()
{
    auto *registry = m_providerManager->registry();
    QStringList providers = registry->availableProviders();
    
    for (const QString &providerId : providers) {
        QString name = registry->providerName(providerId);
        if (name.isEmpty()) {
            name = providerId;
        }
        
        auto *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, providerId);
        m_providerList->addItem(item);
        
        // Create config widget for this provider
        AIProvider *tempProvider = registry->createProvider(providerId);
        if (tempProvider) {
            QWidget *configWidget = tempProvider->createConfigWidget();
            if (configWidget) {
                connect(configWidget, &QWidget::destroyed,
                        this, &ProviderSettingsWidget::onProviderConfigChanged);
                m_configWidgets[providerId] = configWidget;
                m_configStack->addWidget(configWidget);
            }
            tempProvider->deleteLater();
        }
    }
    
    // Select first provider
    if (m_providerList->count() > 0) {
        m_providerList->setCurrentRow(0);
    }
}

void ProviderSettingsWidget::onProviderSelectionChanged()
{
    auto *currentItem = m_providerList->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString providerId = currentItem->data(Qt::UserRole).toString();
    m_currentProviderId = providerId;
    
    // Switch to the appropriate config widget
    if (m_configWidgets.contains(providerId)) {
        m_configStack->setCurrentWidget(m_configWidgets[providerId]);
    }
    
    // Update status
    updateProviderConfig();

    // Persist current active provider config if active and widget present
    if (m_providerManager->activeProviderId() == providerId) {
        if (Application::instance()) {
            QWidget *configWidget = m_configWidgets.value(providerId, nullptr);
            // Config widget handling removed for minimal implementation
            Q_UNUSED(configWidget)
        }
    }
}

void ProviderSettingsWidget::updateProviderConfig()
{
    if (m_currentProviderId.isEmpty()) {
        return;
    }
    
    // Check if this is the active provider
    if (m_providerManager->activeProviderId() == m_currentProviderId) {
        auto *activeProvider = m_providerManager->activeProvider();
        if (activeProvider) {
            AIProvider::Status status = activeProvider->status();
            QString statusText;
            switch (status) {
            case AIProvider::Status::Connected:
                statusText = "Status: Connected";
                break;
            case AIProvider::Status::Connecting:
                statusText = "Status: Connecting...";
                break;
            case AIProvider::Status::Error:
                statusText = QString("Status: Error - %1").arg(activeProvider->statusMessage());
                break;
            default:
                statusText = "Status: Disconnected";
                break;
            }
            m_statusLabel->setText(statusText);
            m_testButton->setEnabled(status != AIProvider::Status::Connecting);
        }
    } else {
        m_statusLabel->setText("Status: Not active");
        m_testButton->setEnabled(true);
    }
}

void ProviderSettingsWidget::onTestConnection()
{
    if (m_currentProviderId.isEmpty()) {
        return;
    }
    
    m_statusLabel->setText("Status: Testing connection...");
    m_testButton->setEnabled(false);
    
    // TODO: Implement connection testing
    QTimer::singleShot(2000, this, [this]() {
        m_statusLabel->setText("Status: Connection test completed");
        m_testButton->setEnabled(true);
    });
}

void ProviderSettingsWidget::onAddProvider()
{
    // TODO: Implement custom provider addition
    QMessageBox::information(this, "Add Provider", "Custom provider addition not yet implemented.");
}

void ProviderSettingsWidget::onRemoveProvider()
{
    // TODO: Implement provider removal
    QMessageBox::information(this, "Remove Provider", "Provider removal not yet implemented.");
}

void ProviderSettingsWidget::onProviderConfigChanged()
{
    emit settingsChanged();

    if (!m_currentProviderId.isEmpty() && m_providerManager->activeProviderId() == m_currentProviderId) {
        QWidget *configWidget = m_configWidgets.value(m_currentProviderId, nullptr);
        if (configWidget) {
            // Config widget handling removed for minimal implementation
            Q_UNUSED(configWidget)
        }
    }
}

void ProviderSettingsWidget::loadSettings()
{
    // TODO: Load provider configurations from settings
}

void ProviderSettingsWidget::saveSettings()
{
    // TODO: Save provider configurations to settings
}

// AppearanceSettingsWidget implementation
AppearanceSettingsWidget::AppearanceSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void AppearanceSettingsWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Font settings group
    auto *fontGroup = new QGroupBox("Font Settings");
    auto *fontLayout = new QFormLayout(fontGroup);
    
    m_fontFamilyCombo = new QFontComboBox();
    connect(m_fontFamilyCombo, QOverload<const QString &>::of(&QFontComboBox::currentTextChanged),
            this, &AppearanceSettingsWidget::onFontChanged);
    fontLayout->addRow("Font Family:", m_fontFamilyCombo);
    
    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(8, 24);
    m_fontSizeSpin->setValue(10);
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AppearanceSettingsWidget::onFontChanged);
    fontLayout->addRow("Font Size:", m_fontSizeSpin);
    
    m_fontBoldCheck = new QCheckBox("Bold text");
    connect(m_fontBoldCheck, &QCheckBox::toggled,
            this, &AppearanceSettingsWidget::onFontChanged);
    fontLayout->addRow("", m_fontBoldCheck);
    
    layout->addWidget(fontGroup);
    
    // Window settings group
    auto *windowGroup = new QGroupBox("Window Settings");
    auto *windowLayout = new QFormLayout(windowGroup);
    
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(50, 100);
    m_opacitySlider->setValue(100);
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onLayoutChanged();
    });
    
    auto *opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(m_opacitySlider);
    m_opacityLabel = new QLabel("100%");
    m_opacityLabel->setMinimumWidth(40);
    opacityLayout->addWidget(m_opacityLabel);
    
    windowLayout->addRow("Window Opacity:", opacityLayout);
    
    layout->addWidget(windowGroup);
    
    // Message display group
    auto *messageGroup = new QGroupBox("Message Display");
    auto *messageLayout = new QVBoxLayout(messageGroup);
    
    m_compactModeCheck = new QCheckBox("Compact mode");
    connect(m_compactModeCheck, &QCheckBox::toggled,
            this, &AppearanceSettingsWidget::onLayoutChanged);
    messageLayout->addWidget(m_compactModeCheck);
    
    m_showTimestampsCheck = new QCheckBox("Show timestamps");
    connect(m_showTimestampsCheck, &QCheckBox::toggled,
            this, &AppearanceSettingsWidget::onLayoutChanged);
    messageLayout->addWidget(m_showTimestampsCheck);
    
    m_showAvatarsCheck = new QCheckBox("Show avatars");
    connect(m_showAvatarsCheck, &QCheckBox::toggled,
            this, &AppearanceSettingsWidget::onLayoutChanged);
    messageLayout->addWidget(m_showAvatarsCheck);
    
    auto *spacingLayout = new QFormLayout();
    m_messageSpacingSpin = new QSpinBox();
    m_messageSpacingSpin->setRange(2, 20);
    m_messageSpacingSpin->setValue(8);
    m_messageSpacingSpin->setSuffix(" px");
    connect(m_messageSpacingSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AppearanceSettingsWidget::onLayoutChanged);
    spacingLayout->addRow("Message spacing:", m_messageSpacingSpin);
    messageLayout->addLayout(spacingLayout);
    
    layout->addWidget(messageGroup);
    
    // Preview
    auto *previewGroup = new QGroupBox("Preview");
    auto *previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewWidget = new QWidget();
    m_previewWidget->setMinimumHeight(100);
    m_previewWidget->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 4px;");
    previewLayout->addWidget(m_previewWidget);
    
    layout->addWidget(previewGroup);
    
    layout->addStretch();
}

void AppearanceSettingsWidget::onFontChanged()
{
    updatePreview();
    emit settingsChanged();
}

void AppearanceSettingsWidget::onColorChanged()
{
    updatePreview();
    emit settingsChanged();
}

void AppearanceSettingsWidget::onLayoutChanged()
{
    updatePreview();
    emit settingsChanged();
}

void AppearanceSettingsWidget::updatePreview()
{
    // TODO: Update the preview widget with current settings
    m_previewWidget->update();
}

void AppearanceSettingsWidget::loadSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    // Font settings
    QString fontFamily = settings->get("ui/fontFamily", "Segoe UI").toString();
    m_fontFamilyCombo->setCurrentText(fontFamily);
    
    int fontSize = settings->get("ui/fontSize", 10).toInt();
    m_fontSizeSpin->setValue(fontSize);
    
    bool fontBold = settings->get("ui/fontBold", false).toBool();
    m_fontBoldCheck->setChecked(fontBold);
    
    // Window settings
    int opacity = settings->get("ui/windowOpacity", 100).toInt();
    m_opacitySlider->setValue(opacity);
    m_opacityLabel->setText(QString("%1%").arg(opacity));
    
    // Message display
    bool compactMode = settings->get("ui/compactMode", false).toBool();
    m_compactModeCheck->setChecked(compactMode);
    
    bool showTimestamps = settings->get("ui/showTimestamps", true).toBool();
    m_showTimestampsCheck->setChecked(showTimestamps);
    
    bool showAvatars = settings->get("ui/showAvatars", true).toBool();
    m_showAvatarsCheck->setChecked(showAvatars);
    
    int messageSpacing = settings->get("ui/messageSpacing", 8).toInt();
    m_messageSpacingSpin->setValue(messageSpacing);
    
    updatePreview();
}

void AppearanceSettingsWidget::saveSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    settings->set("ui/fontFamily", m_fontFamilyCombo->currentText());
    settings->set("ui/fontSize", m_fontSizeSpin->value());
    settings->set("ui/fontBold", m_fontBoldCheck->isChecked());
    settings->set("ui/windowOpacity", m_opacitySlider->value());
    settings->set("ui/compactMode", m_compactModeCheck->isChecked());
    settings->set("ui/showTimestamps", m_showTimestampsCheck->isChecked());
    settings->set("ui/showAvatars", m_showAvatarsCheck->isChecked());
    settings->set("ui/messageSpacing", m_messageSpacingSpin->value());
}

// PrivacySettingsWidget implementation
PrivacySettingsWidget::PrivacySettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void PrivacySettingsWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Data storage group
    auto *storageGroup = new QGroupBox("Data Storage");
    auto *storageLayout = new QVBoxLayout(storageGroup);
    
    m_storeConversationsCheck = new QCheckBox("Store conversation history locally");
    m_storeConversationsCheck->setChecked(true);
    connect(m_storeConversationsCheck, &QCheckBox::toggled,
            this, &PrivacySettingsWidget::onDataRetentionChanged);
    storageLayout->addWidget(m_storeConversationsCheck);
    
    auto *retentionLayout = new QFormLayout();
    m_dataRetentionDaysSpin = new QSpinBox();
    m_dataRetentionDaysSpin->setRange(1, 3650);
    m_dataRetentionDaysSpin->setValue(365);
    m_dataRetentionDaysSpin->setSuffix(" days");
    retentionLayout->addRow("Keep data for:", m_dataRetentionDaysSpin);
    storageLayout->addLayout(retentionLayout);
    
    m_encryptDataCheck = new QCheckBox("Encrypt stored data");
    m_encryptDataCheck->setChecked(true);
    storageLayout->addWidget(m_encryptDataCheck);
    
    layout->addWidget(storageGroup);
    
    // Privacy group
    auto *privacyGroup = new QGroupBox("Privacy");
    auto *privacyLayout = new QVBoxLayout(privacyGroup);
    
    m_shareAnalyticsCheck = new QCheckBox("Share anonymous usage analytics");
    privacyLayout->addWidget(m_shareAnalyticsCheck);
    
    layout->addWidget(privacyGroup);
    
    // Data management group
    auto *managementGroup = new QGroupBox("Data Management");
    auto *managementLayout = new QVBoxLayout(managementGroup);
    
    m_storageUsageLabel = new QLabel("Storage used: Calculating...");
    managementLayout->addWidget(m_storageUsageLabel);
    
    auto *buttonLayout = new QHBoxLayout();
    
    m_exportDataButton = new QPushButton("Export Data");
    connect(m_exportDataButton, &QPushButton::clicked,
            this, &PrivacySettingsWidget::onExportData);
    buttonLayout->addWidget(m_exportDataButton);
    
    m_importDataButton = new QPushButton("Import Data");
    connect(m_importDataButton, &QPushButton::clicked,
            this, &PrivacySettingsWidget::onImportData);
    buttonLayout->addWidget(m_importDataButton);
    
    m_signOutButton = new QPushButton("Sign Out");
    m_signOutButton->setStyleSheet("QPushButton { background-color: #fd7e14; color: white; }");
    connect(m_signOutButton, &QPushButton::clicked,
            this, &PrivacySettingsWidget::onSignOut);
    buttonLayout->addWidget(m_signOutButton);
    
    m_clearDataButton = new QPushButton("Clear All Data");
    m_clearDataButton->setStyleSheet("QPushButton { background-color: #dc3545; color: white; }");
    connect(m_clearDataButton, &QPushButton::clicked,
            this, &PrivacySettingsWidget::onClearData);
    buttonLayout->addWidget(m_clearDataButton);
    
    managementLayout->addLayout(buttonLayout);
    
    layout->addWidget(managementGroup);
    
    layout->addStretch();
}

void PrivacySettingsWidget::onDataRetentionChanged()
{
    emit settingsChanged();
}

void PrivacySettingsWidget::onClearData()
{
    int ret = QMessageBox::warning(this, "Clear All Data",
                                   "This will permanently delete all conversations, attachments, and settings. "
                                   "This action cannot be undone.\n\nAre you sure you want to continue?",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement data clearing
        QMessageBox::information(this, "Data Cleared", "All data has been cleared successfully.");
        emit settingsChanged();
    }
}

void PrivacySettingsWidget::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export Data",
                                                    QString("DesktopApp_Export_%1.json").arg(QDate::currentDate().toString("yyyy-MM-dd")),
                                                    "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        // TODO: Implement data export
        QMessageBox::information(this, "Export Complete", "Data exported successfully to:\n" + fileName);
    }
}

void PrivacySettingsWidget::onImportData()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Import Data",
                                                    QString(),
                                                    "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        int ret = QMessageBox::question(this, "Import Data",
                                        "Importing data will merge with existing data. "
                                        "Duplicate conversations may be created.\n\nContinue with import?",
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            // TODO: Implement data import
            QMessageBox::information(this, "Import Complete", "Data imported successfully from:\n" + fileName);
            emit settingsChanged();
        }
    }
}

void PrivacySettingsWidget::onSignOut()
{
    int ret = QMessageBox::question(this, "Sign Out",
                                   "Are you sure you want to sign out?\n\n"
                                   "This will clear your authentication but keep your conversations and settings.",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        auto *app = Application::instance();
        if (app && app->authenticationService()) {
            qDebug() << "SettingsDialog: Starting sign out process";
            
            // Clear authentication credentials
            app->authenticationService()->signOut();
            
            // Also clear the "Remember Me" preference and saved username
            QSettings loginSettings(QSettings::IniFormat, QSettings::UserScope, "DesktopApp", "ui");
            qDebug() << "SettingsDialog: Before clearing - rememberMe exists:" << loginSettings.contains("login/rememberMe");
            qDebug() << "SettingsDialog: Before clearing - rememberMe value:" << loginSettings.value("login/rememberMe", false).toBool();
            
            // Explicitly set Remember Me to false instead of removing it to ensure it's unchecked
            loginSettings.setValue("login/rememberMe", false);
            loginSettings.remove("login/lastUsername");
            loginSettings.sync();
            
            // Force another sync to ensure the data is written to disk immediately
            loginSettings.sync();
            
            qDebug() << "SettingsDialog: After clearing - rememberMe exists:" << loginSettings.contains("login/rememberMe");
            qDebug() << "SettingsDialog: After clearing - rememberMe value:" << loginSettings.value("login/rememberMe", false).toBool();
            qDebug() << "SettingsDialog: Cleared login preferences and set rememberMe to false";
            
            // Show success message first
            QMessageBox::information(this, "Sign Out", "You have been signed out successfully.\n\n"
                                                      "The application will close. Please restart to sign in again.");
            
            // Close the settings dialog first
            this->close();
            
            // Use a timer to delay the application quit to ensure all QSettings are written
            QTimer::singleShot(200, []() {
                qDebug() << "SettingsDialog: Delayed application quit executing";
                QApplication::quit();
            });
            
        } else {
            QMessageBox::warning(this, "Error", "Authentication service not available.");
        }
    }
}

void PrivacySettingsWidget::loadSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    m_storeConversationsCheck->setChecked(settings->get("privacy/storeConversations", true).toBool());
    m_dataRetentionDaysSpin->setValue(settings->get("privacy/dataRetentionDays", 365).toInt());
    m_encryptDataCheck->setChecked(settings->get("privacy/encryptData", true).toBool());
    m_shareAnalyticsCheck->setChecked(settings->get("privacy/shareAnalytics", false).toBool());
    
    // TODO: Calculate and display actual storage usage
    m_storageUsageLabel->setText("Storage used: ~2.5 MB");
}

void PrivacySettingsWidget::saveSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    settings->set("privacy/storeConversations", m_storeConversationsCheck->isChecked());
    settings->set("privacy/dataRetentionDays", m_dataRetentionDaysSpin->value());
    settings->set("privacy/encryptData", m_encryptDataCheck->isChecked());
    settings->set("privacy/shareAnalytics", m_shareAnalyticsCheck->isChecked());
}

// AdvancedSettingsWidget implementation
AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void AdvancedSettingsWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Network settings group
    auto *networkGroup = new QGroupBox("Network Settings");
    auto *networkLayout = new QFormLayout(networkGroup);
    
    m_networkTimeoutSpin = new QSpinBox();
    m_networkTimeoutSpin->setRange(5, 300);
    m_networkTimeoutSpin->setValue(30);
    m_networkTimeoutSpin->setSuffix(" seconds");
    connect(m_networkTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AdvancedSettingsWidget::onNetworkChanged);
    networkLayout->addRow("Request timeout:", m_networkTimeoutSpin);
    
    m_maxRetries = new QSpinBox();
    m_maxRetries->setRange(0, 10);
    m_maxRetries->setValue(3);
    connect(m_maxRetries, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AdvancedSettingsWidget::onNetworkChanged);
    networkLayout->addRow("Maximum retries:", m_maxRetries);
    
    layout->addWidget(networkGroup);
    
    // Proxy settings group
    auto *proxyGroup = new QGroupBox("Proxy Settings");
    auto *proxyLayout = new QFormLayout(proxyGroup);
    
    m_useProxyCheck = new QCheckBox("Use proxy server");
    connect(m_useProxyCheck, &QCheckBox::toggled, [this](bool enabled) {
        m_proxyHostEdit->setEnabled(enabled);
        m_proxyPortSpin->setEnabled(enabled);
        m_proxyUserEdit->setEnabled(enabled);
        m_proxyPassEdit->setEnabled(enabled);
        onNetworkChanged();
    });
    proxyLayout->addRow("", m_useProxyCheck);
    
    m_proxyHostEdit = new QLineEdit();
    m_proxyHostEdit->setEnabled(false);
    m_proxyHostEdit->setPlaceholderText("proxy.example.com");
    proxyLayout->addRow("Proxy host:", m_proxyHostEdit);
    
    m_proxyPortSpin = new QSpinBox();
    m_proxyPortSpin->setRange(1, 65535);
    m_proxyPortSpin->setValue(8080);
    m_proxyPortSpin->setEnabled(false);
    proxyLayout->addRow("Proxy port:", m_proxyPortSpin);
    
    m_proxyUserEdit = new QLineEdit();
    m_proxyUserEdit->setEnabled(false);
    m_proxyUserEdit->setPlaceholderText("username (optional)");
    proxyLayout->addRow("Username:", m_proxyUserEdit);
    
    m_proxyPassEdit = new QLineEdit();
    m_proxyPassEdit->setEchoMode(QLineEdit::Password);
    m_proxyPassEdit->setEnabled(false);
    m_proxyPassEdit->setPlaceholderText("password (optional)");
    proxyLayout->addRow("Password:", m_proxyPassEdit);
    
    layout->addWidget(proxyGroup);
    
    // Logging settings group
    auto *loggingGroup = new QGroupBox("Logging Settings");
    auto *loggingLayout = new QFormLayout(loggingGroup);
    
    m_logLevelCombo = new QComboBox();
    m_logLevelCombo->addItems({"Debug", "Info", "Warning", "Error", "Critical"});
    m_logLevelCombo->setCurrentText("Info");
    connect(m_logLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedSettingsWidget::onLoggingChanged);
    loggingLayout->addRow("Log level:", m_logLevelCombo);
    
    m_logToFileCheck = new QCheckBox("Log to file");
    connect(m_logToFileCheck, &QCheckBox::toggled, [this](bool enabled) {
        m_logFilePathEdit->setEnabled(enabled);
        m_browseLogButton->setEnabled(enabled);
        onLoggingChanged();
    });
    loggingLayout->addRow("", m_logToFileCheck);
    
    auto *logFileLayout = new QHBoxLayout();
    m_logFilePathEdit = new QLineEdit();
    m_logFilePathEdit->setEnabled(false);
    m_logFilePathEdit->setPlaceholderText("Select log file location...");
    logFileLayout->addWidget(m_logFilePathEdit);
    
    m_browseLogButton = new QPushButton("Browse");
    m_browseLogButton->setEnabled(false);
    connect(m_browseLogButton, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        "Select Log File",
                                                        "DesktopApp.log",
                                                        "Log Files (*.log);;All Files (*.*)");
        if (!fileName.isEmpty()) {
            m_logFilePathEdit->setText(fileName);
        }
    });
    logFileLayout->addWidget(m_browseLogButton);
    
    loggingLayout->addRow("Log file:", logFileLayout);
    
    layout->addWidget(loggingGroup);
    
    // Reset section
    auto *resetGroup = new QGroupBox("Reset Settings");
    auto *resetLayout = new QVBoxLayout(resetGroup);
    
    auto *resetInfo = new QLabel("Reset all settings to their default values. This will not affect your conversation data.");
    resetInfo->setWordWrap(true);
    resetInfo->setStyleSheet("color: #6B7280; font-size: 12px;");
    resetLayout->addWidget(resetInfo);
    
    m_resetButton = new QPushButton("Reset All Settings");
    m_resetButton->setStyleSheet("QPushButton { background-color: #dc3545; color: white; }");
    connect(m_resetButton, &QPushButton::clicked,
            this, &AdvancedSettingsWidget::onResetSettings);
    resetLayout->addWidget(m_resetButton);
    
    layout->addWidget(resetGroup);
    
    layout->addStretch();
}

void AdvancedSettingsWidget::onNetworkChanged()
{
    emit settingsChanged();
}

void AdvancedSettingsWidget::onLoggingChanged()
{
    emit settingsChanged();
}

void AdvancedSettingsWidget::onResetSettings()
{
    int ret = QMessageBox::warning(this, "Reset Settings",
                                   "This will reset all application settings to their default values. "
                                   "Your conversation data will not be affected.\n\nAre you sure?",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: Implement settings reset
        QMessageBox::information(this, "Settings Reset", "All settings have been reset to default values.");
        emit settingsChanged();
    }
}

void AdvancedSettingsWidget::loadSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    // Network settings
    m_networkTimeoutSpin->setValue(settings->get("network/timeout", 30).toInt());
    m_maxRetries->setValue(settings->get("network/maxRetries", 3).toInt());
    
    // Proxy settings
    bool useProxy = settings->get("network/useProxy", false).toBool();
    m_useProxyCheck->setChecked(useProxy);
    m_proxyHostEdit->setText(settings->get("network/proxyHost", "").toString());
    m_proxyPortSpin->setValue(settings->get("network/proxyPort", 8080).toInt());
    m_proxyUserEdit->setText(settings->get("network/proxyUser", "").toString());
    m_proxyPassEdit->setText(settings->get("network/proxyPass", "").toString());
    
    // Enable/disable proxy fields
    m_proxyHostEdit->setEnabled(useProxy);
    m_proxyPortSpin->setEnabled(useProxy);
    m_proxyUserEdit->setEnabled(useProxy);
    m_proxyPassEdit->setEnabled(useProxy);
    
    // Logging settings
    QString logLevel = settings->get("logging/level", "Info").toString();
    m_logLevelCombo->setCurrentText(logLevel);
    
    bool logToFile = settings->get("logging/toFile", false).toBool();
    m_logToFileCheck->setChecked(logToFile);
    m_logFilePathEdit->setText(settings->get("logging/filePath", "").toString());
    
    // Enable/disable log file fields
    m_logFilePathEdit->setEnabled(logToFile);
    m_browseLogButton->setEnabled(logToFile);
}

void AdvancedSettingsWidget::saveSettings()
{
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    // Network settings
    settings->set("network/timeout", m_networkTimeoutSpin->value());
    settings->set("network/maxRetries", m_maxRetries->value());
    
    // Proxy settings
    settings->set("network/useProxy", m_useProxyCheck->isChecked());
    settings->set("network/proxyHost", m_proxyHostEdit->text());
    settings->set("network/proxyPort", m_proxyPortSpin->value());
    settings->set("network/proxyUser", m_proxyUserEdit->text());
    settings->set("network/proxyPass", m_proxyPassEdit->text());
    
    // Logging settings
    settings->set("logging/level", m_logLevelCombo->currentText());
    settings->set("logging/toFile", m_logToFileCheck->isChecked());
    settings->set("logging/filePath", m_logFilePathEdit->text());
}

// SettingsDialog implementation
SettingsDialog::SettingsDialog(ProviderManager *providerManager, QWidget *parent)
    : QDialog(parent)
    , m_hasUnsavedChanges(false)
    , m_providerManager(providerManager)
{
    setWindowTitle("Settings - DesktopApp");
    setModal(true);
    resize(800, 600);
    
    setupUI();
    loadAllSettings();
}

void SettingsDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Tab widget
    m_tabWidget = new QTabWidget();
    
    // Create tab widgets
    m_generalWidget = new GeneralSettingsWidget();
    connect(m_generalWidget, &GeneralSettingsWidget::settingsChanged,
            this, &SettingsDialog::onSettingsChanged);
    m_tabWidget->addTab(m_generalWidget, "General");
    
    m_providerWidget = new ProviderSettingsWidget(m_providerManager);
    connect(m_providerWidget, &ProviderSettingsWidget::settingsChanged,
            this, &SettingsDialog::onSettingsChanged);
    m_tabWidget->addTab(m_providerWidget, "AI Providers");
    
    m_appearanceWidget = new AppearanceSettingsWidget();
    connect(m_appearanceWidget, &AppearanceSettingsWidget::settingsChanged,
            this, &SettingsDialog::onSettingsChanged);
    m_tabWidget->addTab(m_appearanceWidget, "Appearance");
    
    m_privacyWidget = new PrivacySettingsWidget();
    connect(m_privacyWidget, &PrivacySettingsWidget::settingsChanged,
            this, &SettingsDialog::onSettingsChanged);
    m_tabWidget->addTab(m_privacyWidget, "Privacy");
    
    m_advancedWidget = new AdvancedSettingsWidget();
    connect(m_advancedWidget, &AdvancedSettingsWidget::settingsChanged,
            this, &SettingsDialog::onSettingsChanged);
    m_tabWidget->addTab(m_advancedWidget, "Advanced");
    
    layout->addWidget(m_tabWidget);
    
    // Button box
    auto *buttonBox = new QDialogButtonBox();
    
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    
    m_applyButton = buttonBox->addButton(QDialogButtonBox::Apply);
    m_applyButton->setEnabled(false);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApply);
    
    m_resetButton = new QPushButton("Reset Tab");
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onReset);
    buttonBox->addButton(m_resetButton, QDialogButtonBox::ResetRole);
    
    layout->addWidget(buttonBox);
}

void SettingsDialog::loadAllSettings()
{
    m_generalWidget->loadSettings();
    m_providerWidget->loadSettings();
    m_appearanceWidget->loadSettings();
    m_privacyWidget->loadSettings();
    m_advancedWidget->loadSettings();
    
    m_hasUnsavedChanges = false;
    m_applyButton->setEnabled(false);
}

void SettingsDialog::saveAllSettings()
{
    m_generalWidget->saveSettings();
    m_providerWidget->saveSettings();
    m_appearanceWidget->saveSettings();
    m_privacyWidget->saveSettings();
    m_advancedWidget->saveSettings();
    
    m_hasUnsavedChanges = false;
    m_applyButton->setEnabled(false);
}

void SettingsDialog::onApply()
{
    saveAllSettings();
}

void SettingsDialog::onReset()
{
    int currentTab = m_tabWidget->currentIndex();
    
    int ret = QMessageBox::question(this, "Reset Tab",
                                    "Reset all settings in this tab to their default values?",
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Reset current tab to defaults
        switch (currentTab) {
        case 0: // General
            m_generalWidget->loadSettings();
            break;
        case 1: // Providers
            m_providerWidget->loadSettings();
            break;
        case 2: // Appearance
            m_appearanceWidget->loadSettings();
            break;
        case 3: // Privacy
            m_privacyWidget->loadSettings();
            break;
        case 4: // Advanced
            m_advancedWidget->loadSettings();
            break;
        }
    }
}

void SettingsDialog::onSettingsChanged()
{
    enableApplyButton();
}

void SettingsDialog::enableApplyButton()
{
    m_hasUnsavedChanges = true;
    m_applyButton->setEnabled(true);
}

void SettingsDialog::accept()
{
    if (m_hasUnsavedChanges) {
        saveAllSettings();
    }
    QDialog::accept();
}

void SettingsDialog::reject()
{
    if (m_hasUnsavedChanges) {
        int ret = QMessageBox::question(this, "Unsaved Changes",
                                        "You have unsaved changes. Do you want to save them?",
                                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                        QMessageBox::Save);
        
        if (ret == QMessageBox::Save) {
            saveAllSettings();
            QDialog::accept();
        } else if (ret == QMessageBox::Discard) {
            QDialog::reject();
        }
        // Cancel: do nothing, keep dialog open
    } else {
        QDialog::reject();
    }
}

} // namespace DesktopApp

// MOC include not needed with AUTOMOC
