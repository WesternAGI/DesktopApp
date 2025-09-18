#include "MainWindow.h"
#include "ConversationListWidget.h"
#include "MessageThreadWidget.h"
#include "MessageComposer.h"
#include "SettingsDialog.h"
#include "LoginWindow.h"
#include "core/Application.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"
#include "services/SettingsStore.h"
#include "services/AuthenticationService.h"
#include "providers/ProviderManager.h"
#include "services/AudioRecorder.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QShortcut>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QToolBar>
#include <QPropertyAnimation>

namespace DesktopApp {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_mainSplitter(nullptr)
    , m_conversationList(nullptr)
    , m_messageThread(nullptr)
    , m_messageComposer(nullptr)
    , m_statusLabel(nullptr)
    , m_connectionLabel(nullptr)
    , m_modelLabel(nullptr)
{
    setWindowTitle("DesktopApp");
    setMinimumSize(1024, 768);
    resize(1400, 900);

    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupKeyboardShortcuts();
    connectSignals();

    // Apply current theme
    onThemeChanged();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // Create central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Create main layout
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Create main splitter for three-pane layout
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_mainSplitter);

    // Create conversation list (left pane) inside a container to animate width
    m_conversationList = new ConversationListWidget(this);
    m_mainSplitter->addWidget(m_conversationList);
    // Prepare opacity effect for conversation list
    m_sidebarOpacityEffect = new QGraphicsOpacityEffect(m_conversationList);
    m_conversationList->setGraphicsEffect(m_sidebarOpacityEffect);
    // Initial style will be set by applyThemeStyles() after theme manager is ready
    m_sidebarOpacityEffect->setOpacity(0.0);
    m_sidebarWidth = 300; // initial

    // Create right side widget for top bar + message thread and composer
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 25); // Add bottom margin to avoid statusbar overlap
    rightLayout->setSpacing(0);

    // Top bar with sidebar + theme toggle
    m_topBar = new QWidget(rightWidget);
    m_topBar->setObjectName("TopBar");
    // Initial style will be set by applyThemeStyles() after theme manager is ready
    QHBoxLayout *topBarLayout = new QHBoxLayout(m_topBar);
    topBarLayout->setContentsMargins(8,4,8,4);
    topBarLayout->setSpacing(8);
    m_sidebarToggleButton = new QPushButton("≡", m_topBar);
    m_sidebarToggleButton->setFixedSize(40,32);
    m_sidebarToggleButton->setCursor(Qt::PointingHandCursor);
    m_sidebarToggleButton->setToolTip("Show conversations (Ctrl+B)");
    // Initial style will be set by applyThemeStyles() after theme manager is ready
    connect(m_sidebarToggleButton, &QPushButton::clicked, this, &MainWindow::onToggleSidebar);
    topBarLayout->addWidget(m_sidebarToggleButton);
    
    // Add provider selection dropdown
    m_providerCombo = new QComboBox(m_topBar);
    m_providerCombo->addItem("Echo Provider", "echo");
    m_providerCombo->addItem("Backend AI", "backend_ai");
    m_providerCombo->setCurrentText("Backend AI"); // Set default to Backend AI
    m_providerCombo->setMinimumWidth(120);
    m_providerCombo->setToolTip("Select AI Provider");
    topBarLayout->addWidget(m_providerCombo);
    
    topBarLayout->addStretch();
    m_themeToggleButton = new QPushButton(m_topBar);
    m_themeToggleButton->setFixedSize(40,32);
    m_themeToggleButton->setCursor(Qt::PointingHandCursor);
    m_themeToggleButton->setToolTip("Toggle theme (Ctrl+Shift+T)");
    // Initial style will be set by applyThemeStyles() after theme manager is ready
    connect(m_themeToggleButton, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    topBarLayout->addWidget(m_themeToggleButton);
    rightLayout->addWidget(m_topBar);

    // Create message thread (center pane)
    m_messageThread = new MessageThreadWidget(this);
    rightLayout->addWidget(m_messageThread, 1); // Takes most space

    // Create message composer (bottom)
    m_messageComposer = new MessageComposer(this);
    rightLayout->addWidget(m_messageComposer);

    m_mainSplitter->addWidget(rightWidget);

    // Start with sidebar hidden
    m_mainSplitter->setSizes({0, 1200});
    m_sidebarWidth = 0;
    m_sidebarCollapsed = true;
    m_mainSplitter->setCollapsible(0, true);
    m_mainSplitter->setCollapsible(1, false);

    qDebug() << "Main window UI setup complete";
}

void MainWindow::setupMenuBar()
{
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();

    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *newChatAction = fileMenu->addAction(iconRegistry->icon("new-chat"), "&New Conversation");
    newChatAction->setShortcut(QKeySequence::New);
    connect(newChatAction, &QAction::triggered, this, &MainWindow::onNewConversation);

    fileMenu->addSeparator();

    QAction *settingsAction = fileMenu->addAction(iconRegistry->icon("settings"), "&Settings");
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Account menu
    QMenu *accountMenu = menuBar()->addMenu("&Account");
    
    QAction *accountAction = accountMenu->addAction("Account &Management");
    connect(accountAction, &QAction::triggered, this, &MainWindow::onAccountManagement);
    
    accountMenu->addSeparator();
    
    QAction *signOutAction = accountMenu->addAction("&Sign Out");
    connect(signOutAction, &QAction::triggered, this, &MainWindow::onSignOut);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *aboutAction = helpMenu->addAction("&About DesktopApp");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel, 1);

    m_connectionLabel = new QLabel("Offline");
    statusBar()->addPermanentWidget(m_connectionLabel);

    m_modelLabel = new QLabel("Echo Provider");
    statusBar()->addPermanentWidget(m_modelLabel);
}

void MainWindow::setupKeyboardShortcuts()
{
    // Global shortcuts
    new QShortcut(QKeySequence("Escape"), this, [this]() {
        if (m_messageComposer) {
            m_messageComposer->setFocus();
        }
    });

    new QShortcut(QKeySequence("Ctrl+/"), this, [this]() {
        // Show keyboard shortcuts dialog
        QMessageBox::information(this, "Keyboard Shortcuts",
            "Ctrl+N - New Conversation\n"
            "Ctrl+, - Settings\n"
            "Ctrl+Shift+T - Toggle Theme\n"
            "Escape - Focus Message Composer\n"
            "Ctrl+/ - Show This Help");
    });
}

void MainWindow::connectSignals()
{
    auto *app = Application::instance();
    
    // Connect theme changes
    connect(app->themeManager(), &ThemeManager::themeChanged,
            this, &MainWindow::onThemeChanged);

    // Connect conversation list to message thread
    connect(m_conversationList, &ConversationListWidget::conversationSelected,
            m_messageThread, &MessageThreadWidget::loadConversation);

    // Connect message composer to message thread
    connect(m_messageComposer, &MessageComposer::messageSent,
            m_messageThread, &MessageThreadWidget::addUserMessage);

    // Connect provider selection to provider manager
    connect(m_messageComposer, &MessageComposer::providerChanged, this, [app](const QString &providerId) {
        auto *providerManager = app->providerManager();
        if (providerManager) {
            providerManager->setActiveProvider(providerId);
        }
    });
    
    // Connect top bar provider combo to provider manager
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, app](int index) {
        QString providerId = m_providerCombo->itemData(index).toString();
        auto *providerManager = app->providerManager();
        if (providerManager) {
            providerManager->setActiveProvider(providerId);
        }
    });

    // Set initial provider selection in both combos
    auto *providerManager = app->providerManager();
    if (providerManager) {
        QString activeProviderId = providerManager->activeProviderId();
        m_messageComposer->setCurrentProvider(activeProviderId);
        
        // Set top bar combo to match active provider
        for (int i = 0; i < m_providerCombo->count(); ++i) {
            if (m_providerCombo->itemData(i).toString() == activeProviderId) {
                m_providerCombo->setCurrentIndex(i);
                break;
            }
        }
        
        // Update both combos when provider changes externally
        connect(providerManager, &ProviderManager::activeProviderChanged,
                this, [this](const QString &providerId) {
                    // Update message composer combo
                    m_messageComposer->setCurrentProvider(providerId);
                    
                    // Update top bar combo
                    for (int i = 0; i < m_providerCombo->count(); ++i) {
                        if (m_providerCombo->itemData(i).toString() == providerId) {
                            m_providerCombo->blockSignals(true); // Prevent recursion
                            m_providerCombo->setCurrentIndex(i);
                            m_providerCombo->blockSignals(false);
                            break;
                        }
                    }
                });
    }

    // Connect message thread responses back to conversation list
    connect(m_messageThread, &MessageThreadWidget::conversationUpdated,
            m_conversationList, &ConversationListWidget::refreshConversations);

    // Notify on audio device changes if recorder available via application (future accessor)
    if (auto *rec = app->audioRecorder()) {
        connect(rec, &AudioRecorder::deviceChanged, this, [this](const QString &desc){
            statusBar()->showMessage(desc, 5000);
        });
    }
}

void MainWindow::setSidebarWidth(int w)
{
    if (!m_mainSplitter) return;
    m_sidebarWidth = w;
    QList<int> sizes = m_mainSplitter->sizes();
    if (sizes.size() >= 2) {
        int total = sizes.at(0) + sizes.at(1);
        sizes[0] = w;
        sizes[1] = std::max(100, total - w);
        m_mainSplitter->setSizes(sizes);
    }
}

void MainWindow::onToggleSidebar()
{
    if (!m_sidebarGroup) {
    m_sidebarAnimation = new QPropertyAnimation(this, "sidebarWidth");
    m_sidebarAnimation->setDuration(400);
    m_sidebarAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_sidebarFade = new QPropertyAnimation(m_sidebarOpacityEffect, "opacity");
    m_sidebarFade->setDuration(360);
    m_sidebarFade->setEasingCurve(QEasingCurve::OutCubic);
        m_sidebarGroup = new QParallelAnimationGroup(this);
        m_sidebarGroup->addAnimation(m_sidebarAnimation);
        m_sidebarGroup->addAnimation(m_sidebarFade);
    }
    if (m_sidebarGroup->state() == QAbstractAnimation::Running) return;

    if (!m_sidebarCollapsed) {
        m_sidebarStoredWidth = m_sidebarWidth;
        m_sidebarAnimation->setStartValue(m_sidebarWidth);
        m_sidebarAnimation->setEndValue(0);
        m_sidebarFade->setStartValue(1.0);
        m_sidebarFade->setEndValue(0.0);
        m_sidebarCollapsed = true;
        if (m_sidebarToggleButton) m_sidebarToggleButton->setToolTip("Show conversations (Ctrl+B)");
    } else {
    int target = m_sidebarStoredWidth > 40 ? m_sidebarStoredWidth : 320;
        m_sidebarAnimation->setStartValue(m_sidebarWidth);
        m_sidebarAnimation->setEndValue(target);
        m_sidebarFade->setStartValue(0.0);
        m_sidebarFade->setEndValue(1.0);
        m_sidebarCollapsed = false;
        if (m_sidebarToggleButton) m_sidebarToggleButton->setToolTip("Hide conversations (Ctrl+B)");
    }
    m_sidebarGroup->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Save window geometry
    auto *app = Application::instance();
    auto *settings = app->settingsStore();
    
    settings->setValue("window/geometry", saveGeometry());
    settings->setValue("window/state", saveState());
    settings->setValue("window/splitter", m_mainSplitter->saveState());

    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle global key events
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onNewConversation()
{
    if (m_conversationList) {
        m_conversationList->createNewConversation();
    }
}

void MainWindow::onOpenSettings()
{
    if (!m_settingsDialog) {
    auto *app = Application::instance();
    m_settingsDialog = std::make_unique<SettingsDialog>(app->providerManager(), this);
    }
    
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::onToggleTheme()
{
    startThemeFadeTransition();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About DesktopApp",
        "<h3>DesktopApp 1.0.0</h3>"
        "<p>A modern AI chat desktop application.</p>"
        "<p>Built with Qt6 and C++17.</p>"
        "<p>Copyright © 2025 DesktopApp Project</p>"
        "<p><b>Features:</b></p>"
        "<ul>"
        "<li>Cross-platform desktop chat interface</li>"
        "<li>Multiple AI provider support</li>"
        "<li>Local data storage and privacy</li>"
        "<li>Light and dark themes</li>"
        "<li>Keyboard shortcuts and accessibility</li>"
        "</ul>");
}

void MainWindow::onThemeChanged()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    applyThemeStyles();
    
    // Force update all widgets to ensure theme is applied
    this->update();
    if (m_conversationList) m_conversationList->update();
    if (m_messageThread) m_messageThread->update();
    if (m_topBar) m_topBar->update();
    
    m_statusLabel->setText(QString("Theme: %1").arg(themeManager->currentThemeString()));
    qDebug() << "Applied theme styles:" << themeManager->currentThemeString();
}

void MainWindow::applyThemeStyles()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    auto *iconRegistry = app->iconRegistry();
    const auto &tokens = themeManager->tokens();
    bool dark = themeManager->currentTheme() == ThemeManager::Dark;
    setWindowIcon(iconRegistry->icon("chat"));

    // Apply theme colors consistently using ThemeManager tokens
    if (m_topBar) {
        m_topBar->setStyleSheet(QString("#TopBar { background:%1; border-bottom:1px solid %2; }")
            .arg(tokens.surface.name())
            .arg(tokens.border.name()));
    }
    if (m_conversationList) {
        m_conversationList->setStyleSheet(QString("background:%1; border-right:1px solid %2; color:%3;")
            .arg(tokens.background.name())
            .arg(tokens.border.name())
            .arg(tokens.text.name()));
        
        // Update conversation list's internal theme styles
        m_conversationList->updateThemeStyles();
    }
    if (m_messageThread) {
        m_messageThread->setStyleSheet(QString("background:%1; color:%2;")
            .arg(tokens.background.name())
            .arg(tokens.text.name()));
    }
    if (statusBar()) {
        statusBar()->setStyleSheet(QString("QStatusBar { background:%1; color:%2; border-top:1px solid %3; }")
            .arg(tokens.surface.name())
            .arg(tokens.textMuted.name())
            .arg(tokens.border.name()));
    }

    // Buttons using theme tokens
    if (m_themeToggleButton) {
        m_themeToggleButton->setText(dark ? "🌙" : "☀");
        m_themeToggleButton->setStyleSheet(QString(
            "QPushButton { font-size:18px; border:1px solid %1; border-radius:%2px; background:%3; color:%4; }"
            "QPushButton:hover { background:%5; }"
            "QPushButton:pressed { background:%6; }")
            .arg(tokens.border.name())
            .arg(tokens.radiusMedium)
            .arg(tokens.surface.name())
            .arg(tokens.text.name())
            .arg(tokens.surfaceHover.name())
            .arg(tokens.border.name()));
    }
    if (m_sidebarToggleButton) {
        m_sidebarToggleButton->setStyleSheet(QString(
            "QPushButton { font-size:18px; font-weight:600; border:1px solid %1; border-radius:%2px; background:%3; color:%4; }"
            "QPushButton:hover { background:%5; border-color:%6; }"
            "QPushButton:pressed { background:%7; }")
            .arg(tokens.border.name())
            .arg(tokens.radiusMedium)
            .arg(tokens.surface.name())
            .arg(tokens.text.name())
            .arg(tokens.surfaceHover.name())
            .arg(tokens.primary.name())
            .arg(tokens.border.name()));
    }
}

void MainWindow::startThemeFadeTransition()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    ThemeManager::Theme currentTheme = themeManager->currentTheme();
    ThemeManager::Theme newTheme = (currentTheme == ThemeManager::Light) ? ThemeManager::Dark : ThemeManager::Light;

    // Create overlay (QLabel with pixmap) lazily
    if (!m_themeFadeOverlay) {
        m_themeFadeOverlay = new QLabel(this);
        m_themeFadeOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_themeFadeOverlay->setScaledContents(true);
        m_themeFadeEffect = new QGraphicsOpacityEffect(m_themeFadeOverlay);
        m_themeFadeOverlay->setGraphicsEffect(m_themeFadeEffect);
        m_themeFadeAnimation = new QPropertyAnimation(m_themeFadeEffect, "opacity", this);
        m_themeFadeAnimation->setDuration(2250); // fade out duration (old theme dissolving)
        m_themeFadeAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        connect(m_themeFadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            if (m_themeFadeEffect->opacity() <= 0.0) m_themeFadeOverlay->hide();
        });
    }

    // Grab current window (old theme)
    QPixmap oldFrame = this->grab();
    m_themeFadeOverlay->setPixmap(oldFrame);
    m_themeFadeOverlay->setGeometry(rect());
    m_themeFadeOverlay->raise();
    m_themeFadeOverlay->show();
    m_themeFadeEffect->setOpacity(1.0);

    // Switch to new theme immediately under the overlay (hidden)
    themeManager->setTheme(newTheme);
    // Force immediate polish/update
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // Start fading old captured frame so new theme reveals beneath
    m_themeFadeAnimation->stop();
    m_themeFadeAnimation->setStartValue(1.0);
    m_themeFadeAnimation->setEndValue(0.0);
    m_themeFadeAnimation->start();
}

void MainWindow::onAccountManagement()
{
    // Show login window in "account management" mode
    LoginWindow *loginWindow = new LoginWindow(this);
    loginWindow->setAttribute(Qt::WA_DeleteOnClose);
    
    // Set window title to indicate it's for account management
    loginWindow->setWindowTitle("Account Management - DesktopApp");
    
    // Show the login window as a modal dialog
    loginWindow->exec();
}

void MainWindow::onSignOut()
{
    int result = QMessageBox::question(this, "Sign Out", 
        "Are you sure you want to sign out?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        // Sign out from authentication service
        // Note: In a real implementation, you'd get the auth service from the application
        // auto *app = Application::instance();
        // AuthenticationService *authService = app->authenticationService();
        // authService->signOut();
        
        // Close the main window and show login window
        this->close();
        
        // Show login window for re-authentication
        LoginWindow *loginWindow = new LoginWindow();
        loginWindow->setAttribute(Qt::WA_DeleteOnClose);
        
        connect(loginWindow, &LoginWindow::loginSuccessful, [this](const QString &username, const QString &token) {
            Q_UNUSED(username)
            Q_UNUSED(token)
            // Show the main window again
            this->show();
        });
        
        connect(loginWindow, &QDialog::rejected, qApp, &QApplication::quit);
        
        loginWindow->show();
    }
}

} // namespace DesktopApp
