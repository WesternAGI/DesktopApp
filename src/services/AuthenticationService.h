#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QCryptographicHash>
#include <QSettings>
#include <QDateTime>
#include <QElapsedTimer>

namespace DesktopApp {

/**
 * @brief Authentication service for user management
 */
class AuthenticationService : public QObject
{
    Q_OBJECT

public:
    explicit AuthenticationService(QObject *parent = nullptr);
    ~AuthenticationService();

    enum AuthError {
        NoError = 0,
        NetworkError,
        InvalidCredentials,
        UserNotFound,
        UserExists,
        InvalidToken,
        TwoFactorRequired,
        AccountLocked,
        ServerError
    };

    struct UserProfile {
        QString id;                 // backend user_id (stringified)
        QString username;           // login username
        QString phoneNumber;        // phone_number from backend
        QString firstName;          // (unused by backend currently)
        QString lastName;           // (unused)
        QString avatarUrl;
        QDateTime createdAt;
        QDateTime lastLoginAt;
        bool phoneVerified = false;
        bool twoFactorEnabled = false;
        QString role;               // role string from backend
    };

    struct AuthSession {
        QString accessToken;
        QString tokenType; // bearer
        QDateTime expiresAt; // absolute expiry
        QString userId;
        QString username;
        QString role;
        bool isValid() const { return !accessToken.isEmpty() && QDateTime::currentDateTimeUtc() < expiresAt; }
    };

public slots:
    void signIn(const QString &usernameOrPhone, const QString &password, bool rememberMe = false);
    void registerUser(const QString &username, const QString &phoneNumber, const QString &password);
    // Backward compatibility: older UI code may still call (first,last,phone,password)
    void registerUser(const QString &firstName, const QString &lastName, const QString &phoneNumber, const QString &password);
    void signOut();
    void resetPassword(const QString &phoneNumber);
    void verifyTwoFactor(const QString &code);
    void refreshToken();
    void updateProfile(const UserProfile &profile);
    void changePassword(const QString &currentPassword, const QString &newPassword);
    void enableTwoFactor();
    void disableTwoFactor();


    // Session management
    bool isAuthenticated() const;
    QString getCurrentToken() const;
    UserProfile getCurrentUser() const;
    void restoreSession();

signals:
    void authenticationFinished(bool success, const QString &message);
    void registrationFinished(bool success, const QString &message);
    void passwordResetFinished(bool success, const QString &message);
    void twoFactorVerificationFinished(bool success, const QString &message);
    void profileUpdated(const UserProfile &profile);
    void sessionExpired();
    void userSignedOut();

private slots:
    void onTokenRefreshTimer();

private:
    void makeAuthRequest(const QString &endpoint, const QJsonObject &data);
    void handleAuthResponse(const QJsonObject &response);
    void saveCredentials(const QString &token, const QString &refreshToken, bool rememberMe);
    void clearCredentials();
    QString hashPassword(const QString &password, const QString &salt = QString());
    QString generateSalt();
    void startTokenRefreshTimer();
    void stopTokenRefreshTimer();

    // Remote auth helpers
    void performApiRequest(const QString &endpoint, const QJsonObject &payload, const QString &purpose);
    void processLoginResponse(int status, const QJsonObject &obj, const QString &networkError);
    void processRegisterResponse(int status, const QJsonObject &obj, const QString &networkError);
    void logRequestStart(const QString &id, const QString &endpoint, const QJsonObject &payload) const;
    void logRequestEnd(const QString &id, const QString &endpoint, int statusCode, qint64 msec, const QString &error = QString()) const;
    void scheduleExpiryLogout(int secondsUntilExpiry);
    QString redact(const QString &value) const;

    // Backward compatibility (legacy signatures still referenced in generated code until clean rebuild)
    void processLoginResponse(const QJsonObject &obj) { processLoginResponse(200, obj, QString()); }
    void processRegisterResponse(const QJsonObject &obj) { processRegisterResponse(201, obj, QString()); }

private:
    QNetworkAccessManager *m_networkManager;
    QSettings *m_settings;
    QTimer *m_tokenRefreshTimer;
    
    AuthSession m_session;
    UserProfile m_currentUser;
    bool m_isAuthenticated;
    
    // Legacy tokens kept to satisfy old compiled references; will be removed after full clean
    QString m_currentToken;
    QString m_refreshToken;
    
    // Request tracking
    struct PendingRequest { QString purpose; QString endpoint; QDateTime startTime; };
    QHash<QNetworkReply*, PendingRequest> m_pending;
    QString m_apiBase = "https://web-production-d7d37.up.railway.app"; // backend base URL
};

} // namespace DesktopApp
