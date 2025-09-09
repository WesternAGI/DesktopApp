#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QTimer>

class QTabWidget;
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QSlider;
class QLabel;
class QPushButton;
class QGroupBox;
class QListWidget;
class QStackedWidget;
class QScrollArea;

namespace DesktopApp {

class AIProvider;
class ProviderManager;

/**
 * @brief General settings tab
 */
class GeneralSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onThemeChanged();
    void onLanguageChanged();
    void onStartupChanged();

private:
    void setupUI();
    
    QComboBox *m_themeCombo;
    QComboBox *m_languageCombo;
    QCheckBox *m_startMinimizedCheck;
    QCheckBox *m_autoStartCheck;
    QCheckBox *m_checkUpdatesCheck;
    QCheckBox *m_analyticsCheck;
    QSpinBox *m_maxConversationsSpin;
    QSpinBox *m_autoSaveIntervalSpin;
};

/**
 * @brief Provider settings tab
 */
class ProviderSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProviderSettingsWidget(ProviderManager *providerManager, QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onProviderSelectionChanged();
    void onTestConnection();
    void onProviderConfigChanged();
    void onAddProvider();
    void onRemoveProvider();

private:
    void setupUI();
    void updateProviderConfig();
    void populateProviderList();
    
    ProviderManager *m_providerManager;
    QListWidget *m_providerList;
    QStackedWidget *m_configStack;
    QPushButton *m_testButton;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QLabel *m_statusLabel;
    
    QHash<QString, QWidget*> m_configWidgets;
    QString m_currentProviderId;
};

/**
 * @brief Appearance settings tab
 */
class AppearanceSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AppearanceSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onFontChanged();
    void onColorChanged();
    void onLayoutChanged();

private:
    void setupUI();
    void updatePreview();
    
    QComboBox *m_fontFamilyCombo;
    QSpinBox *m_fontSizeSpin;
    QCheckBox *m_fontBoldCheck;
    QSlider *m_opacitySlider;
    QLabel *m_opacityLabel;
    QCheckBox *m_compactModeCheck;
    QCheckBox *m_showTimestampsCheck;
    QCheckBox *m_showAvatarsCheck;
    QSpinBox *m_messageSpacingSpin;
    QWidget *m_previewWidget;
};

/**
 * @brief Privacy settings tab
 */
class PrivacySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacySettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onDataRetentionChanged();
    void onClearData();
    void onExportData();
    void onImportData();

private:
    void setupUI();
    
    QCheckBox *m_storeConversationsCheck;
    QSpinBox *m_dataRetentionDaysSpin;
    QCheckBox *m_encryptDataCheck;
    QCheckBox *m_shareAnalyticsCheck;
    QPushButton *m_clearDataButton;
    QPushButton *m_exportDataButton;
    QPushButton *m_importDataButton;
    QLabel *m_storageUsageLabel;
};

/**
 * @brief Advanced settings tab
 */
class AdvancedSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onNetworkChanged();
    void onLoggingChanged();
    void onResetSettings();

private:
    void setupUI();
    
    QSpinBox *m_networkTimeoutSpin;
    QSpinBox *m_maxRetries;
    QCheckBox *m_useProxyCheck;
    QLineEdit *m_proxyHostEdit;
    QSpinBox *m_proxyPortSpin;
    QLineEdit *m_proxyUserEdit;
    QLineEdit *m_proxyPassEdit;
    QComboBox *m_logLevelCombo;
    QCheckBox *m_logToFileCheck;
    QLineEdit *m_logFilePathEdit;
    QPushButton *m_browseLogButton;
    QPushButton *m_resetButton;
};

/**
 * @brief Main settings dialog
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(ProviderManager *providerManager, QWidget *parent = nullptr);

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onApply();
    void onReset();
    void onSettingsChanged();

private:
    void setupUI();
    void loadAllSettings();
    void saveAllSettings();
    void enableApplyButton();
    
    QTabWidget *m_tabWidget;
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    
    GeneralSettingsWidget *m_generalWidget;
    ProviderSettingsWidget *m_providerWidget;
    AppearanceSettingsWidget *m_appearanceWidget;
    PrivacySettingsWidget *m_privacyWidget;
    AdvancedSettingsWidget *m_advancedWidget;
    
    bool m_hasUnsavedChanges;
    ProviderManager *m_providerManager;
};

} // namespace DesktopApp
