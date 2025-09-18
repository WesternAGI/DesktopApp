#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <memory>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>

namespace DesktopApp {

class ConversationListWidget;
class MessageThreadWidget;
class MessageComposer;
class SettingsDialog;
class LoginWindow;
class AuthenticationService;

/**
 * @brief Main application window with three-pane layout
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(int sidebarWidth READ sidebarWidth WRITE setSidebarWidth)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onNewConversation();
    void onOpenSettings();
    void onToggleTheme();
    void onAbout();
    void onThemeChanged();
    void onToggleSidebar();
    void onAccountManagement();
    void onSignOut();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupKeyboardShortcuts();
    void connectSignals();

    // Sidebar animation helpers
    int sidebarWidth() const { return m_sidebarWidth; }
    void setSidebarWidth(int w);

    // UI Components
    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    ConversationListWidget *m_conversationList;
    MessageThreadWidget *m_messageThread;
    MessageComposer *m_messageComposer;
    
    // Dialogs
    std::unique_ptr<SettingsDialog> m_settingsDialog;
    
    // Status bar widgets
    QLabel *m_statusLabel;
    QLabel *m_connectionLabel;
    QLabel *m_modelLabel;
    QPushButton *m_sidebarToggleButton {nullptr};
    QPushButton *m_themeToggleButton {nullptr};
    QWidget *m_topBar {nullptr}; // Added top bar pointer
    QComboBox *m_providerCombo {nullptr}; // Provider selection dropdown

    // Sidebar animation state
    int m_sidebarWidth {300};
    int m_sidebarStoredWidth {300};
    bool m_sidebarCollapsed {false};
    QPropertyAnimation *m_sidebarAnimation {nullptr};
    QPropertyAnimation *m_sidebarFade {nullptr};
    QGraphicsOpacityEffect *m_sidebarOpacityEffect {nullptr};
    QParallelAnimationGroup *m_sidebarGroup {nullptr};

    void applyThemeStyles(); // theme style helper
    void startThemeFadeTransition();

    // Theme transition
    class QLabel *m_themeFadeOverlay {nullptr};
    QGraphicsOpacityEffect *m_themeFadeEffect {nullptr};
    QPropertyAnimation *m_themeFadeAnimation {nullptr};
};

} // namespace DesktopApp
