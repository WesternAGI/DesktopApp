#include "LoginWindow.h"
#include "services/AuthenticationService.h"
#include "../theme/ThemeManager.h"
#include "../core/Application.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopServices>
#include <QUrl>
#include <QRegularExpression>
#include <QMessageBox>
#include <QEasingCurve>
#include <QGraphicsBlurEffect>
#include <QSplitter>
#include <QTextBrowser>
#include <QScrollBar>
#include <QStandardPaths>
#include <QDir>
#include <QStyle>
#include <QDebug>

namespace GadAI {

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_stackedWidget(nullptr)
    , m_currentPage(SignInPage)
    , m_authState(Idle)
    , m_authService(nullptr)
    , m_primaryColor("#3B82F6")      // Blue
    , m_secondaryColor("#6B7280")    // Gray
    , m_errorColor("#EF4444")        // Red
    , m_successColor("#10B981")      // Green
    , m_backgroundColor("#F9FAFB")   // Light gray
    , m_cardColor("#FFFFFF")         // White
    , m_textColor("#111827")         // Dark gray
    , m_placeholderColor("#9CA3AF")  // Light gray
{
    setWindowTitle("Welcome to GadAI");
    setModal(true);
    // Keep window resizable but ensure base size without needing scroll
    setMinimumSize(480, 640);
    resize(520, 640);
    setSizeGripEnabled(true);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // Center the window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Initialize authentication service
    m_authService = new AuthenticationService(this);
    
    setupUI();
    setupAnimations();
    applyModernStyling();
    
    // Connect authentication service signals
    connect(m_authService, &AuthenticationService::authenticationFinished,
            this, &LoginWindow::onAuthenticationFinished);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create main container with padding
    QWidget *containerWidget = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);
    containerLayout->setContentsMargins(36, 32, 36, 32);
    containerLayout->setSpacing(16);
    
    // Setup header
    setupHeaderSection();
    containerLayout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
    containerLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    // Subtitle removed per request (keep object for potential future use but hidden)
    m_subtitleLabel->hide();
    
    // Create stacked widget for different pages
    m_stackedWidget = new QStackedWidget;
    setupSignInPage();
    setupRegisterPage();
    setupForgotPasswordPage();
    setupTwoFactorPage();
    
    containerLayout->addWidget(m_stackedWidget);
    
    // Social login removed
    
    // Message label for errors/success
    m_messageLabel = new QLabel;
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->hide();
    containerLayout->addWidget(m_messageLabel);
    
    // Progress bar
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 0); // Indeterminate
    m_progressBar->hide();
    containerLayout->addWidget(m_progressBar);
    
    containerLayout->addStretch();
    
    // Directly add container (no scrolling desired)
    containerWidget->setMaximumWidth(540);
    m_mainLayout->addWidget(containerWidget, 0, Qt::AlignHCenter);
}

void LoginWindow::setupHeaderSection()
{
    // Logo
    m_logoLabel = new QLabel;
    // Create a simple text logo since we don't have image files
    m_logoLabel->setText("🤖");
    m_logoLabel->setStyleSheet("font-size: 48px;");
    m_logoLabel->setAlignment(Qt::AlignCenter);
    
    // Title
    m_titleLabel = new QLabel("Welcome to GadAI");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    
    // Subtitle
    m_subtitleLabel = new QLabel("Your intelligent conversation companion");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
}

void LoginWindow::setupSignInPage()
{
    m_signInPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_signInPage);
    layout->setSpacing(12);
    
    // Form container with proper spacing
    QWidget *formWidget = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(8); // compact vertical rhythm
    formLayout->setContentsMargins(0,0,0,0);
    
    // Username field
    QLabel *usernameLabel = new QLabel("Username");
    usernameLabel->setObjectName("fieldLabel");
    m_signInUsernameEdit = new QLineEdit;
    m_signInUsernameEdit->setPlaceholderText("Enter your username");

    formLayout->addWidget(usernameLabel);
    formLayout->addWidget(m_signInUsernameEdit);
    
    // Password field
    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("fieldLabel");
    
    QWidget *passwordContainer = new QWidget;
    QHBoxLayout *passwordLayout = new QHBoxLayout(passwordContainer);
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(8);
    
    m_signInPasswordEdit = new QLineEdit;
    m_signInPasswordEdit->setPlaceholderText("Enter your password");
    m_signInPasswordEdit->setEchoMode(QLineEdit::Password);
    
    m_showSignInPasswordButton = new QPushButton("👁");
    m_showSignInPasswordButton->setFixedSize(32, 32);
    m_showSignInPasswordButton->setCheckable(true);
    
    passwordLayout->addWidget(m_signInPasswordEdit);
    passwordLayout->addWidget(m_showSignInPasswordButton);
    
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordContainer);
    
    layout->addWidget(formWidget);
    
    // Options
    QHBoxLayout *optionsLayout = new QHBoxLayout;
    m_rememberMeCheckBox = new QCheckBox("Remember me");
    m_forgotPasswordButton = new QPushButton("Forgot password?");
    m_forgotPasswordButton->setFlat(true);
    
    optionsLayout->addWidget(m_rememberMeCheckBox);
    optionsLayout->addStretch();
    optionsLayout->addWidget(m_forgotPasswordButton);
    layout->addLayout(optionsLayout);
    
    // Sign in button
    m_signInButton = new QPushButton("Sign In");
    m_signInButton->setMinimumHeight(44);
    layout->addWidget(m_signInButton);
    
    // Switch to register
    QHBoxLayout *switchLayout = new QHBoxLayout;
    QLabel *switchLabel = new QLabel("Don't have an account?");
    m_switchToRegisterButton = new QPushButton("Create one");
    m_switchToRegisterButton->setFlat(true);
    
    switchLayout->addStretch();
    switchLayout->addWidget(switchLabel);
    switchLayout->addWidget(m_switchToRegisterButton);
    switchLayout->addStretch();
    layout->addLayout(switchLayout);
    
    // Connect signals
    connect(m_signInButton, &QPushButton::clicked, this, &LoginWindow::onSignInClicked);
    connect(m_showSignInPasswordButton, &QPushButton::clicked, this, &LoginWindow::onShowPassword);
    connect(m_forgotPasswordButton, &QPushButton::clicked, this, &LoginWindow::onSwitchToForgotPassword);
    connect(m_switchToRegisterButton, &QPushButton::clicked, this, &LoginWindow::onSwitchToRegister);
    connect(m_rememberMeCheckBox, &QCheckBox::toggled, this, &LoginWindow::onRememberMeToggled);
    connect(m_signInUsernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_signInPasswordEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    
    // Enter key handling
    connect(m_signInUsernameEdit, &QLineEdit::returnPressed, m_signInButton, &QPushButton::click);
    connect(m_signInPasswordEdit, &QLineEdit::returnPressed, m_signInButton, &QPushButton::click);
    
    m_stackedWidget->addWidget(m_signInPage);
}

void LoginWindow::setupRegisterPage()
{
    m_registerPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_registerPage);
    layout->setSpacing(12);
    layout->setContentsMargins(0,0,0,0);
    
    // Form container
    QWidget *formWidget = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(8);
    formLayout->setContentsMargins(0,0,0,0);
    
    // Name fields in horizontal layout
    QWidget *nameContainer = new QWidget;
    QHBoxLayout *nameLayout = new QHBoxLayout(nameContainer);
    nameLayout->setSpacing(8);
    nameLayout->setContentsMargins(0,0,0,0);
    
    QWidget *firstNameWidget = new QWidget;
    QVBoxLayout *firstNameLayout = new QVBoxLayout(firstNameWidget);
    firstNameLayout->setContentsMargins(0, 0, 0, 0);
    firstNameLayout->setSpacing(4);
    
    QLabel *firstNameLabel = new QLabel("First Name");
    firstNameLabel->setObjectName("fieldLabel");
    m_firstNameEdit = new QLineEdit;
    m_firstNameEdit->setPlaceholderText("Enter your first name");
    
    firstNameLayout->addWidget(firstNameLabel);
    firstNameLayout->addWidget(m_firstNameEdit);
    
    QWidget *lastNameWidget = new QWidget;
    QVBoxLayout *lastNameLayout = new QVBoxLayout(lastNameWidget);
    lastNameLayout->setContentsMargins(0, 0, 0, 0);
    lastNameLayout->setSpacing(4);
    
    QLabel *lastNameLabel = new QLabel("Last Name");
    lastNameLabel->setObjectName("fieldLabel");
    m_lastNameEdit = new QLineEdit;
    m_lastNameEdit->setPlaceholderText("Enter your last name");
    
    lastNameLayout->addWidget(lastNameLabel);
    lastNameLayout->addWidget(m_lastNameEdit);
    
    nameLayout->addWidget(firstNameWidget);
    nameLayout->addWidget(lastNameWidget);
    
    formLayout->addWidget(nameContainer);
    
    // Username field
    QLabel *usernameRegLabel = new QLabel("Username");
    usernameRegLabel->setObjectName("fieldLabel");
    m_registerUsernameEdit = new QLineEdit;
    m_registerUsernameEdit->setPlaceholderText("Choose a unique username");

    formLayout->addWidget(usernameRegLabel);
    formLayout->addWidget(m_registerUsernameEdit);
    
    // Phone field
    QLabel *phoneRegLabel = new QLabel("Phone Number");
    phoneRegLabel->setObjectName("fieldLabel");
    m_registerPhoneEdit = new QLineEdit;
    m_registerPhoneEdit->setPlaceholderText("Enter phone (e.g. +15551234567)");

    formLayout->addWidget(phoneRegLabel);
    formLayout->addWidget(m_registerPhoneEdit);
    
    // Password field
    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("fieldLabel");
    
    QWidget *passwordContainer = new QWidget;
    QHBoxLayout *passwordLayout = new QHBoxLayout(passwordContainer);
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(8);
    
    m_registerPasswordEdit = new QLineEdit;
    m_registerPasswordEdit->setPlaceholderText("Create a strong password");
    m_registerPasswordEdit->setEchoMode(QLineEdit::Password);
    
    m_showRegisterPasswordButton = new QPushButton("👁");
    m_showRegisterPasswordButton->setFixedSize(36, 36); // slightly smaller to reduce height
    m_showRegisterPasswordButton->setCheckable(true);
    
    passwordLayout->addWidget(m_registerPasswordEdit);
    passwordLayout->addWidget(m_showRegisterPasswordButton);
    
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordContainer);
    
    // Confirm password field
    QLabel *confirmPasswordLabel = new QLabel("Confirm Password");
    confirmPasswordLabel->setObjectName("fieldLabel");
    
    QWidget *confirmPasswordContainer = new QWidget;
    QHBoxLayout *confirmPasswordLayout = new QHBoxLayout(confirmPasswordContainer);
    confirmPasswordLayout->setContentsMargins(0, 0, 0, 0);
    confirmPasswordLayout->setSpacing(8);
    
    m_confirmPasswordEdit = new QLineEdit;
    m_confirmPasswordEdit->setPlaceholderText("Confirm your password");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    
    m_showConfirmPasswordButton = new QPushButton("👁");
    m_showConfirmPasswordButton->setFixedSize(36, 36);
    m_showConfirmPasswordButton->setCheckable(true);
    
    confirmPasswordLayout->addWidget(m_confirmPasswordEdit);
    confirmPasswordLayout->addWidget(m_showConfirmPasswordButton);
    
    formLayout->addWidget(confirmPasswordLabel);
    formLayout->addWidget(confirmPasswordContainer);
    
    layout->addWidget(formWidget);
    
    // Terms and conditions
    m_acceptTermsCheckBox = new QCheckBox("I accept the Terms of Service and Privacy Policy");
    layout->addWidget(m_acceptTermsCheckBox);
    
    // Register button
    m_registerButton = new QPushButton("Create Account");
    m_registerButton->setMinimumHeight(44);
    layout->addWidget(m_registerButton);
    
    // Switch to sign in
    QHBoxLayout *switchLayout = new QHBoxLayout;
    QLabel *switchLabel = new QLabel("Already have an account?");
    m_switchToSignInButton = new QPushButton("Sign in");
    m_switchToSignInButton->setFlat(true);
    
    switchLayout->addStretch();
    switchLayout->addWidget(switchLabel);
    switchLayout->addWidget(m_switchToSignInButton);
    switchLayout->addStretch();
    layout->addLayout(switchLayout);
    
    // Connect signals
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(m_showRegisterPasswordButton, &QPushButton::clicked, this, &LoginWindow::onShowPassword);
    connect(m_showConfirmPasswordButton, &QPushButton::clicked, this, &LoginWindow::onShowConfirmPassword);
    connect(m_switchToSignInButton, &QPushButton::clicked, this, &LoginWindow::onSwitchToSignIn);
    connect(m_acceptTermsCheckBox, &QCheckBox::toggled, this, &LoginWindow::onTermsAccepted);
    
    // Input validation
    connect(m_firstNameEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_lastNameEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_registerPhoneEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_registerPasswordEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_confirmPasswordEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    
    m_stackedWidget->addWidget(m_registerPage);
}

void LoginWindow::setupForgotPasswordPage()
{
    m_forgotPasswordPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_forgotPasswordPage);
    layout->setSpacing(16);
    
    // Instructions
    QLabel *instructionsLabel = new QLabel(
        "Enter your phone number and we'll send you a reset code (demo)."
    );
    instructionsLabel->setWordWrap(true);
    layout->addWidget(instructionsLabel);
    
    // Phone field
    QLabel *phoneResetLabel = new QLabel("Phone Number");
    phoneResetLabel->setObjectName("fieldLabel");
    m_resetPhoneEdit = new QLineEdit;
    m_resetPhoneEdit->setPlaceholderText("Enter your phone number");
    layout->addWidget(phoneResetLabel);
    layout->addWidget(m_resetPhoneEdit);
    
    // Reset button
    m_resetPasswordButton = new QPushButton("Send Reset Link");
    m_resetPasswordButton->setMinimumHeight(44);
    layout->addWidget(m_resetPasswordButton);
    
    // Back to sign in
    m_backToSignInButton = new QPushButton("← Back to Sign In");
    m_backToSignInButton->setFlat(true);
    layout->addWidget(m_backToSignInButton);
    
    layout->addStretch();
    
    // Connect signals
    connect(m_resetPasswordButton, &QPushButton::clicked, this, &LoginWindow::onForgotPasswordClicked);
    connect(m_backToSignInButton, &QPushButton::clicked, this, &LoginWindow::onBackToSignIn);
    connect(m_resetPhoneEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_resetPhoneEdit, &QLineEdit::returnPressed, m_resetPasswordButton, &QPushButton::click);
    
    m_stackedWidget->addWidget(m_forgotPasswordPage);
}

void LoginWindow::setupTwoFactorPage()
{
    m_twoFactorPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_twoFactorPage);
    layout->setSpacing(16);
    
    // Info label
    m_twoFactorInfoLabel = new QLabel(
        "We've sent a verification code to your phone (demo). Please enter it below."
    );
    m_twoFactorInfoLabel->setWordWrap(true);
    layout->addWidget(m_twoFactorInfoLabel);
    
    // Code field
    QLabel *codeLabel = new QLabel("Verification Code");
    codeLabel->setObjectName("fieldLabel");
    m_twoFactorCodeEdit = new QLineEdit;
    m_twoFactorCodeEdit->setPlaceholderText("Enter 6-digit code");
    m_twoFactorCodeEdit->setMaxLength(6);
    layout->addWidget(codeLabel);
    layout->addWidget(m_twoFactorCodeEdit);
    
    // Verify button
    m_verifyCodeButton = new QPushButton("Verify Code");
    m_verifyCodeButton->setMinimumHeight(44);
    layout->addWidget(m_verifyCodeButton);
    
    // Resend code
    m_resendCodeButton = new QPushButton("Resend Code");
    m_resendCodeButton->setFlat(true);
    layout->addWidget(m_resendCodeButton);
    
    layout->addStretch();
    
    // Connect signals
    connect(m_verifyCodeButton, &QPushButton::clicked, this, &LoginWindow::onSignInClicked);
    connect(m_resendCodeButton, &QPushButton::clicked, this, &LoginWindow::onForgotPasswordClicked);
    connect(m_twoFactorCodeEdit, &QLineEdit::textChanged, this, &LoginWindow::onInputChanged);
    connect(m_twoFactorCodeEdit, &QLineEdit::returnPressed, m_verifyCodeButton, &QPushButton::click);
    
    m_stackedWidget->addWidget(m_twoFactorPage);
}


void LoginWindow::setupAnimations()
{
    // Opacity effect for transitions
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_stackedWidget->setGraphicsEffect(m_opacityEffect);
    
    // Page transition animation
    m_pageAnimation = new QPropertyAnimation(this, "currentPage", this);
    m_pageAnimation->setDuration(300);
    m_pageAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Opacity animation
    m_opacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_opacityAnimation->setDuration(150);
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Group animations
    m_transitionGroup = new QParallelAnimationGroup(this);
    m_transitionGroup->addAnimation(m_opacityAnimation);
    
    connect(m_transitionGroup, &QParallelAnimationGroup::finished,
            this, &LoginWindow::onPageTransitionFinished);
}

void LoginWindow::applyModernStyling()
{
    // Main window styling
    setStyleSheet(QString(R"(
        LoginWindow {
            background: %1;
            border: 1px solid #E5E7EB;
            border-radius: 12px;
        }
        
        QLabel {
            color: %2;
            font-weight: 500;
        }
        
        QLabel#title {
            font-size: 24px;
            font-weight: 700;
            color: %3;
            margin: 8px 0;
        }
        
        QLabel#subtitle {
            font-size: 14px;
            color: %4;
            margin-bottom: 16px;
        }

        QLabel#fieldLabel, QLabel[fieldLabel="true"], QLabel.fieldLabel, QLabel[fieldLabel] {
            font-size: 12px;
            font-weight: 600;
            letter-spacing: 0.3px;
            margin: 2px 0 2px 2px;
            text-transform: none;
        }
        
        QLineEdit {
            background: %5;
            border: 2px solid #E5E7EB;
            border-radius: 8px;
            padding: 10px 14px;
            font-size: 14px;
            color: %6;
            min-height: 20px;
        }
        
        QLineEdit:focus {
            border-color: %7;
            background: %8;
        }
        
        QLineEdit::placeholder {
            color: %9;
        }
        
        QPushButton {
            background: %10;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: 600;
            min-height: 20px;
        }
        
        QPushButton:hover {
            background: #2563EB;
        }
        
        QPushButton:pressed {
            background: #1D4ED8;
        }
        
        QPushButton:disabled {
            background: #9CA3AF;
            color: #6B7280;
        }
        
        QPushButton[flat="true"] {
            background: transparent;
            color: %11;
            padding: 4px 8px;
        }
        
        QPushButton[flat="true"]:hover {
            color: #2563EB;
            background: #F3F4F6;
        }
        
        QCheckBox {
            color: %12;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #D1D5DB;
            border-radius: 4px;
            background: %13;
        }
        
        QCheckBox::indicator:checked {
            background: %14;
            border-color: %15;
        }
        
        QProgressBar {
            border: none;
            border-radius: 4px;
            background: #F3F4F6;
            height: 8px;
        }
        
        QProgressBar::chunk {
            background: %16;
            border-radius: 4px;
        }
        
        QLabel#error {
            color: %17;
            background: #FEF2F2;
            border: 1px solid #FECACA;
            border-radius: 6px;
            padding: 8px 12px;
        }
        
        QLabel#success {
            color: %18;
            background: #F0FDF4;
            border: 1px solid #BBF7D0;
            border-radius: 6px;
            padding: 8px 12px;
        }
    )").arg(
        m_backgroundColor,     // 1
        m_textColor,          // 2
        m_textColor,          // 3
        m_secondaryColor,     // 4
        m_cardColor,          // 5
        m_textColor,          // 6
        m_primaryColor,       // 7
        m_cardColor,          // 8
        m_placeholderColor,   // 9
        m_primaryColor,       // 10
        m_primaryColor,       // 11
        m_textColor,          // 12
        m_cardColor,          // 13
        m_primaryColor,       // 14
        m_primaryColor,       // 15
        m_primaryColor,       // 16
        m_errorColor,         // 17
        m_successColor        // 18
    ));
    
    // Set object names for specific styling
    m_titleLabel->setObjectName("title");
    m_subtitleLabel->setObjectName("subtitle");
}

// Slot implementations

void LoginWindow::showSignInPage()
{
    animateToPage(SignInPage);
}

void LoginWindow::showRegisterPage()
{
    animateToPage(RegisterPage);
}

void LoginWindow::showForgotPasswordPage()
{
    animateToPage(ForgotPasswordPage);
}

void LoginWindow::showTwoFactorPage()
{
    animateToPage(TwoFactorPage);
}

void LoginWindow::onSignInClicked()
{
    if (m_currentPage == TwoFactorPage) {
        // Handle two-factor verification
        QString code = m_twoFactorCodeEdit->text().trimmed();
        if (code.length() != 6) {
            showError("Please enter a valid 6-digit verification code.");
            return;
        }
        // TODO: Verify the code
        emit loginSuccessful(m_pendingUsername, "dummy_token");
        accept();
        return;
    }
    
    if (!validateSignInForm()) {
        return;
    }
    
    performSignIn();
}

void LoginWindow::onRegisterClicked()
{
    if (!validateRegisterForm()) {
        return;
    }
    
    performRegistration();
}

void LoginWindow::onForgotPasswordClicked()
{
    QString phone = m_resetPhoneEdit->text().trimmed();
    if (!validatePhone(phone)) {
        showError("Please enter a valid phone number (E.164 format).");
        return;
    }
    
    performPasswordReset();
}

void LoginWindow::onSwitchToRegister()
{
    clearMessages();
    showRegisterPage();
}

void LoginWindow::onSwitchToSignIn()
{
    clearMessages();
    showSignInPage();
}

void LoginWindow::onSwitchToForgotPassword()
{
    clearMessages();
    showForgotPasswordPage();
}

void LoginWindow::onBackToSignIn()
{
    clearMessages();
    showSignInPage();
}

void LoginWindow::onShowPassword()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QLineEdit *passwordEdit = nullptr;
    if (button == m_showSignInPasswordButton) {
        passwordEdit = m_signInPasswordEdit;
    } else if (button == m_showRegisterPasswordButton) {
        passwordEdit = m_registerPasswordEdit;
    }
    
    if (passwordEdit) {
        if (button->isChecked()) {
            passwordEdit->setEchoMode(QLineEdit::Normal);
            button->setText("🙈");
        } else {
            passwordEdit->setEchoMode(QLineEdit::Password);
            button->setText("👁");
        }
    }
}

void LoginWindow::onShowConfirmPassword()
{
    if (m_showConfirmPasswordButton->isChecked()) {
        m_confirmPasswordEdit->setEchoMode(QLineEdit::Normal);
        m_showConfirmPasswordButton->setText("🙈");
    } else {
        m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
        m_showConfirmPasswordButton->setText("👁");
    }
}

void LoginWindow::onRememberMeToggled(bool checked)
{
    // TODO: Save remember me preference
    Q_UNUSED(checked)
}

void LoginWindow::onTermsAccepted(bool checked)
{
    Q_UNUSED(checked)
    updateButtonStates();
}


void LoginWindow::onInputChanged()
{
    clearMessages();
    updateButtonStates();
}

void LoginWindow::onPageTransitionFinished()
{
    m_stackedWidget->setCurrentIndex(m_currentPage);
    m_opacityAnimation->setStartValue(0.0);
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
}

void LoginWindow::onAuthenticationFinished(bool success, const QString &message)
{
    setAuthState(success ? Success : Error);
    
    if (success) {
        showSuccess(message);
        // Emit loginSuccessful so the main window can appear (main.cpp listens for this)
        QString usernameUsed;
        QString token;
        if (m_authService) {
            token = m_authService->getCurrentToken();
            usernameUsed = m_authService->getCurrentUser().username; // may be empty if not populated yet
        }
        if (usernameUsed.isEmpty()) {
            // Fallback to what user typed in the sign-in field
            if (m_signInUsernameEdit) usernameUsed = m_signInUsernameEdit->text().trimmed();
        }
        emit loginSuccessful(usernameUsed, token);
        // Don't call accept() - let the main.cpp handle the success signal
        // This avoids the dialog event loop issue
    } else {
        showError(message);
    }
}

// Helper methods

void LoginWindow::setCurrentPage(int page)
{
    m_currentPage = page;
}

void LoginWindow::animateToPage(int page)
{
    if (page == m_currentPage) return;
    
    m_opacityAnimation->setStartValue(1.0);
    m_opacityAnimation->setEndValue(0.0);
    m_pageAnimation->setStartValue(m_currentPage);
    m_pageAnimation->setEndValue(page);
    
    // Update current page for the transition finished handler
    m_currentPage = page;
    
    m_transitionGroup->start();
}

bool LoginWindow::validateSignInForm()
{
    QString username = m_signInUsernameEdit->text().trimmed();
    QString password = m_signInPasswordEdit->text();

    if (username.isEmpty()) {
        showError("Please enter your username.");
        m_signInUsernameEdit->setFocus();
        return false;
    }

    if (password.isEmpty()) {
        showError("Please enter your password.");
        m_signInPasswordEdit->setFocus();
        return false;
    }

    return true;
}

bool LoginWindow::validateRegisterForm()
{
    QString firstName = m_firstNameEdit->text().trimmed();
    QString lastName = m_lastNameEdit->text().trimmed();
    QString username = m_registerUsernameEdit->text().trimmed();
    QString phone = m_registerPhoneEdit->text().trimmed();
    QString password = m_registerPasswordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    
    if (firstName.isEmpty()) {
        showError("Please enter your first name.");
        m_firstNameEdit->setFocus();
        return false;
    }
    
    if (lastName.isEmpty()) {
        showError("Please enter your last name.");
        m_lastNameEdit->setFocus();
        return false;
    }
    
    if (username.isEmpty()) {
        showError("Please enter a username.");
        m_registerUsernameEdit->setFocus();
        return false;
    }
    
    if (username.length() < 3) {
        showError("Username must be at least 3 characters long.");
        m_registerUsernameEdit->setFocus();
        return false;
    }
    
    if (!validatePhone(phone)) {
        showError("Please enter a valid phone number (E.164 format).");
        m_registerPhoneEdit->setFocus();
        return false;
    }
    
    if (!validatePassword(password)) {
        showError("Password must be at least 8 characters long and contain uppercase, lowercase, number, and special character.");
        m_registerPasswordEdit->setFocus();
        return false;
    }
    
    if (password != confirmPassword) {
        showError("Passwords do not match.");
        m_confirmPasswordEdit->setFocus();
        return false;
    }
    
    if (!m_acceptTermsCheckBox->isChecked()) {
        showError("Please accept the Terms of Service and Privacy Policy.");
        return false;
    }
    
    return true;
}

bool LoginWindow::validatePhone(const QString &phone)
{
    static QRegularExpression phoneRegex(R"(^\+[1-9][0-9]{7,14}$)");
    return phoneRegex.match(phone).hasMatch();
}

bool LoginWindow::validatePassword(const QString &password)
{
    if (password.length() < 8) {
        return false;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (const QChar &ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isDigit()) hasDigit = true;
        else if (!ch.isLetterOrNumber()) hasSpecial = true;
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

void LoginWindow::showError(const QString &message)
{
    m_messageLabel->setText(message);
    m_messageLabel->setObjectName("error");
    m_messageLabel->style()->unpolish(m_messageLabel);
    m_messageLabel->style()->polish(m_messageLabel);
    m_messageLabel->show();
}

void LoginWindow::showSuccess(const QString &message)
{
    m_messageLabel->setText(message);
    m_messageLabel->setObjectName("success");
    m_messageLabel->style()->unpolish(m_messageLabel);
    m_messageLabel->style()->polish(m_messageLabel);
    m_messageLabel->show();
}

void LoginWindow::clearMessages()
{
    m_messageLabel->hide();
    m_messageLabel->clear();
}

void LoginWindow::setAuthState(AuthState state)
{
    m_authState = state;
    
    switch (state) {
    case Idle:
        m_progressBar->hide();
        break;
    case Authenticating:
        m_progressBar->show();
        break;
    case Success:
    case Error:
        m_progressBar->hide();
        break;
    }
    
    updateButtonStates();
}

void LoginWindow::performSignIn()
{
    setAuthState(Authenticating);
    
    QString username = m_signInUsernameEdit->text().trimmed();
    QString password = m_signInPasswordEdit->text();
    bool rememberMe = m_rememberMeCheckBox->isChecked();
    
    // TODO: Replace with actual authentication
    if (m_authService) {
        m_authService->signIn(username, password, rememberMe);
    } else {
        // Simulate authentication
        QTimer::singleShot(2000, [this, username]() {
            if (username == "demo" && m_signInPasswordEdit->text() == "demo123") {
                m_pendingUsername = username;
                showTwoFactorPage(); // Simulate 2FA requirement
                setAuthState(Idle);
            } else {
                onAuthenticationFinished(false, "Invalid username or password.");
            }
        });
    }
}

void LoginWindow::performRegistration()
{
    setAuthState(Authenticating);
    
    QString firstName = m_firstNameEdit->text().trimmed();
    QString lastName = m_lastNameEdit->text().trimmed();
    QString phone = m_registerPhoneEdit->text().trimmed();
    QString password = m_registerPasswordEdit->text();
    
    // TODO: Replace with actual registration
    if (m_authService) {
        QString username;
        if (!firstName.isEmpty() || !lastName.isEmpty()) {
            username = (firstName + lastName).toLower().replace(" ", "");
            if (username.isEmpty()) username = phone;
        } else {
            username = phone;
        }
        m_authService->registerUser(username, phone, password);
    } else {
        // Simulate registration
        QTimer::singleShot(2000, [this, phone]() {
            emit registrationSuccessful(phone);
            onAuthenticationFinished(true, "Account created successfully! Please verify your phone (demo).");
        });
    }
}

void LoginWindow::performPasswordReset()
{
    setAuthState(Authenticating);
    
    QString phone = m_resetPhoneEdit->text().trimmed();
    
    // TODO: Replace with actual password reset
    QTimer::singleShot(2000, [this]() {
        onAuthenticationFinished(true, "Password reset code sent to your phone (demo).");
    });
}

void LoginWindow::updateButtonStates()
{
    bool isAuthenticating = (m_authState == Authenticating);
    
    // Sign in page
    if (m_signInButton) {
    bool canSignIn = !m_signInUsernameEdit->text().trimmed().isEmpty() && 
                        !m_signInPasswordEdit->text().isEmpty() && 
                        !isAuthenticating;
        m_signInButton->setEnabled(canSignIn);
        m_signInButton->setText(isAuthenticating ? "Signing In..." : "Sign In");
    }
    
    // Register page
    if (m_registerButton) {
        bool canRegister = !m_firstNameEdit->text().trimmed().isEmpty() &&
                          !m_lastNameEdit->text().trimmed().isEmpty() &&
                          !m_registerPhoneEdit->text().trimmed().isEmpty() &&
                          !m_registerPasswordEdit->text().isEmpty() &&
                          !m_confirmPasswordEdit->text().isEmpty() &&
                          m_acceptTermsCheckBox->isChecked() &&
                          !isAuthenticating;
        m_registerButton->setEnabled(canRegister);
        m_registerButton->setText(isAuthenticating ? "Creating Account..." : "Create Account");
    }
    
    // Forgot password page
    if (m_resetPasswordButton) {
    bool canReset = !m_resetPhoneEdit->text().trimmed().isEmpty() && !isAuthenticating;
        m_resetPasswordButton->setEnabled(canReset);
        m_resetPasswordButton->setText(isAuthenticating ? "Sending..." : "Send Reset Link");
    }
    
    // Two factor page
    if (m_verifyCodeButton) {
        bool canVerify = m_twoFactorCodeEdit->text().length() == 6 && !isAuthenticating;
        m_verifyCodeButton->setEnabled(canVerify);
        m_verifyCodeButton->setText(isAuthenticating ? "Verifying..." : "Verify Code");
    }
}

void LoginWindow::resetForms()
{
    // Clear all form fields
    m_signInUsernameEdit->clear();
    m_signInPasswordEdit->clear();
    m_firstNameEdit->clear();
    m_lastNameEdit->clear();
    m_registerPhoneEdit->clear();
    m_registerPasswordEdit->clear();
    m_confirmPasswordEdit->clear();
    m_resetPhoneEdit->clear();
    m_twoFactorCodeEdit->clear();
    
    // Reset checkboxes
    m_rememberMeCheckBox->setChecked(false);
    m_acceptTermsCheckBox->setChecked(false);
    
    // Reset password visibility
    m_showSignInPasswordButton->setChecked(false);
    m_showRegisterPasswordButton->setChecked(false);
    m_showConfirmPasswordButton->setChecked(false);
    onShowPassword();
    onShowConfirmPassword();
    
    // Clear messages
    clearMessages();
    
    // Reset state
    setAuthState(Idle);
}

} // namespace GadAI
