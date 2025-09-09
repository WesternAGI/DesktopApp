#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QFrame>
#include <QScrollArea>
#include <QPixmap>
#include <QMovie>

namespace GadAI {

class AuthenticationService;

/**
 * @brief Modern login window with sign-in and registration functionality
 */
class LoginWindow : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage)

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    enum Page {
        SignInPage = 0,
        RegisterPage = 1,
        ForgotPasswordPage = 2,
        TwoFactorPage = 3
    };

    enum AuthState {
        Idle,
        Authenticating,
        Success,
        Error
    };

public slots:
    void showSignInPage();
    void showRegisterPage();
    void showForgotPasswordPage();
    void showTwoFactorPage();
    
private slots:
    void onSignInClicked();
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onSwitchToRegister();
    void onSwitchToSignIn();
    void onSwitchToForgotPassword();
    void onBackToSignIn();
    void onShowPassword();
    void onShowConfirmPassword();
    void onRememberMeToggled(bool checked);
    void onTermsAccepted(bool checked);
    void onInputChanged();
    
    // Animation slots
    void onPageTransitionFinished();
    void onAuthenticationFinished(bool success, const QString &message);

signals:
    void loginSuccessful(const QString &username, const QString &token);
    void registrationSuccessful(const QString &username);
    void authenticationFailed(const QString &error);

private:
    void setupUI();
    void setupSignInPage();
    void setupRegisterPage();
    void setupForgotPasswordPage();
    void setupTwoFactorPage();
    void setupHeaderSection();
    void applyModernStyling();
    void setupAnimations();
    
    // Page transition
    void setCurrentPage(int page);
    int currentPage() const { return m_currentPage; }
    void animateToPage(int page);
    
    // Validation
    bool validateSignInForm();
    bool validateRegisterForm();
    bool validatePhone(const QString &phone);
    bool validatePassword(const QString &password);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void clearMessages();
    
    // Authentication
    void setAuthState(AuthState state);
    void performSignIn();
    void performRegistration();
    void performPasswordReset();
    
    // UI state management
    void updateButtonStates();
    void resetForms();

private:
    // Main layout
    QVBoxLayout *m_mainLayout;
    QStackedWidget *m_stackedWidget;
    
    // Pages
    QWidget *m_signInPage;
    QWidget *m_registerPage;
    QWidget *m_forgotPasswordPage;
    QWidget *m_twoFactorPage;
    
    // Header
    QLabel *m_logoLabel;
    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;
    
    // Sign In Form
    QLineEdit *m_signInPhoneEdit;
    QLineEdit *m_signInPasswordEdit;
    QPushButton *m_signInButton;
    QPushButton *m_showSignInPasswordButton;
    QCheckBox *m_rememberMeCheckBox;
    QPushButton *m_forgotPasswordButton;
    QPushButton *m_switchToRegisterButton;
    
    // Register Form
    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QLineEdit *m_registerPhoneEdit;
    QLineEdit *m_registerPasswordEdit;
    QLineEdit *m_confirmPasswordEdit;
    QPushButton *m_registerButton;
    QPushButton *m_showRegisterPasswordButton;
    QPushButton *m_showConfirmPasswordButton;
    QCheckBox *m_acceptTermsCheckBox;
    QPushButton *m_switchToSignInButton;
    
    // Forgot Password Form
    QLineEdit *m_resetPhoneEdit;
    QPushButton *m_resetPasswordButton;
    QPushButton *m_backToSignInButton;
    
    // Two Factor Form
    QLineEdit *m_twoFactorCodeEdit;
    QPushButton *m_verifyCodeButton;
    QPushButton *m_resendCodeButton;
    QLabel *m_twoFactorInfoLabel;
    
    // Social login buttons removed
    
    // Status and feedback
    QLabel *m_messageLabel;
    QProgressBar *m_progressBar;
    QFrame *m_loadingFrame;
    QMovie *m_loadingMovie;
    QLabel *m_loadingLabel;
    
    // State
    int m_currentPage;
    AuthState m_authState;
    QString m_pendingUsername;
    
    // Animations
    QPropertyAnimation *m_pageAnimation;
    QPropertyAnimation *m_opacityAnimation;
    QParallelAnimationGroup *m_transitionGroup;
    QGraphicsOpacityEffect *m_opacityEffect;
    
    // Services
    AuthenticationService *m_authService;
    
    // Styling
    QString m_primaryColor;
    QString m_secondaryColor;
    QString m_errorColor;
    QString m_successColor;
    QString m_backgroundColor;
    QString m_cardColor;
    QString m_textColor;
    QString m_placeholderColor;
};

} // namespace GadAI
