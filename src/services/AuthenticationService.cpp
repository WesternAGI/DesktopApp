#include "AuthenticationService.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QUuid>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

namespace DesktopApp {

AuthenticationService::AuthenticationService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, 
                              "DesktopApp", "authentication", this))
    , m_tokenRefreshTimer(new QTimer(this))
    , m_isAuthenticated(false)
    , m_useLocalAuth(true) // Use local auth for demo purposes
{
    connect(m_tokenRefreshTimer, &QTimer::timeout, this, &AuthenticationService::onTokenRefreshTimer);
    
    // Initialize demo users
    initializeDemoUsers();
    
    // Try to restore previous session
    restoreSession();
}

AuthenticationService::~AuthenticationService() = default;

void AuthenticationService::initializeDemoUsers()
{
    // Create demo user for testing
    QJsonObject demoUser;
    demoUser["id"] = "demo-user-1";
    demoUser["phoneNumber"] = "+15550000001"; // E.164 demo
    demoUser["username"] = "demo";
    demoUser["firstName"] = "Demo";
    demoUser["lastName"] = "User";
    demoUser["passwordHash"] = hashPassword("demo123", "demosalt");
    demoUser["salt"] = "demosalt";
    demoUser["phoneVerified"] = true;
    demoUser["twoFactorEnabled"] = false; // Disable 2FA for simplicity
    demoUser["createdAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Only store the single demo user
    m_demoUsers["+15550000001"] = demoUser;
    m_demoUsers["demo"] = demoUser; // Also allow username lookup
}

void AuthenticationService::signIn(const QString &usernameOrPhone, const QString &password, bool rememberMe)
{
    Q_UNUSED(rememberMe)
    qDebug() << "AuthenticationService: Remote sign in for" << usernameOrPhone;
    m_useLocalAuth = false; // switch to remote mode
    QJsonObject payload;
    payload["username"] = usernameOrPhone;
    payload["password"] = password;
    performApiRequest("/token", payload, "login");
}

void AuthenticationService::registerUser(const QString &username, const QString &phoneNumber, const QString &password)
{
    qDebug() << "AuthenticationService: Remote registration for" << username;
    m_useLocalAuth = false;
    QJsonObject payload;
    payload["username"] = username;
    payload["password"] = password;
    payload["phone_number"] = phoneNumber;
    performApiRequest("/register", payload, "register");
}

// Backward compatibility wrapper (first,last,phone,password)
void AuthenticationService::registerUser(const QString &firstName, const QString &lastName, const QString &phoneNumber, const QString &password)
{
    QString username = (firstName + lastName).toLower().replace(" ", "");
    if (username.isEmpty()) username = phoneNumber;
    registerUser(username, phoneNumber, password);
}

void AuthenticationService::signOut()
{
    qDebug() << "AuthenticationService: Signing out user";
    
    m_isAuthenticated = false;
    m_session = AuthSession{};
    m_currentUser = UserProfile{};
    clearCredentials();
    stopTokenRefreshTimer();
    
    emit userSignedOut();
}

void AuthenticationService::resetPassword(const QString &phoneNumber)
{
    qDebug() << "AuthenticationService: Password reset requested for" << phoneNumber;
    
    if (m_useLocalAuth) {
        // Simulate password reset
        if (m_demoUsers.contains(phoneNumber)) {
            emit passwordResetFinished(true, "Password reset code sent to your phone");
        } else {
            emit passwordResetFinished(false, "No account found with this phone number");
        }
        return;
    }
    
    QJsonObject requestData;
    requestData["phoneNumber"] = phoneNumber;
    
    makeAuthRequest("/auth/reset-password", requestData);
}

void AuthenticationService::verifyTwoFactor(const QString &code)
{
    qDebug() << "AuthenticationService: Verifying 2FA code";
    
    if (m_useLocalAuth) {
        // For demo purposes, accept "123456" as valid code
        if (code == "123456") {
            m_isAuthenticated = true;
            startTokenRefreshTimer();
            emit twoFactorVerificationFinished(true, "Two-factor authentication successful");
        } else {
            emit twoFactorVerificationFinished(false, "Invalid verification code");
        }
        return;
    }
    
    QJsonObject payload;
    payload["code"] = code;
    performApiRequest("/verify-2fa", payload, "verify2fa");
}

void AuthenticationService::refreshToken()
{
    // No refresh endpoint; rely on expiry scheduling
}

void AuthenticationService::updateProfile(const UserProfile &profile)
{
    m_currentUser = profile;
    
    if (m_useLocalAuth) {
        saveLocalUser(profile, ""); // Don't update password hash
        emit profileUpdated(profile);
        return;
    }
    
    QJsonObject requestData;
    requestData["firstName"] = profile.firstName;
    requestData["lastName"] = profile.lastName;
    requestData["phoneNumber"] = profile.phoneNumber;
    
    makeAuthRequest("/auth/update-profile", requestData);
}

void AuthenticationService::changePassword(const QString &currentPassword, const QString &newPassword)
{
    QJsonObject requestData;
    requestData["currentPassword"] = currentPassword;
    requestData["newPassword"] = newPassword;
    
    makeAuthRequest("/auth/change-password", requestData);
}

void AuthenticationService::enableTwoFactor()
{
    makeAuthRequest("/auth/enable-2fa", QJsonObject());
}

void AuthenticationService::disableTwoFactor()
{
    makeAuthRequest("/auth/disable-2fa", QJsonObject());
}


bool AuthenticationService::isAuthenticated() const
{
    return m_session.isValid();
}

QString AuthenticationService::getCurrentToken() const
{
    return m_session.accessToken;
}

AuthenticationService::UserProfile AuthenticationService::getCurrentUser() const
{
    return m_currentUser;
}

void AuthenticationService::restoreSession()
{
    QString token = m_settings->value("auth/accessToken").toString();
    QDateTime expiresAt = m_settings->value("auth/expiresAt").toDateTime();
    QString userId = m_settings->value("auth/userId").toString();
    QString username = m_settings->value("auth/username").toString();
    QString role = m_settings->value("auth/role").toString();
    if (!token.isEmpty() && expiresAt.isValid() && QDateTime::currentDateTimeUtc() < expiresAt) {
        m_session.accessToken = token;
        m_session.tokenType = "bearer";
        m_session.expiresAt = expiresAt;
        m_session.userId = userId;
        m_session.username = username;
        m_session.role = role;
        m_isAuthenticated = true;
        scheduleExpiryLogout(QDateTime::currentDateTimeUtc().secsTo(expiresAt));
        qDebug() << "AuthenticationService: Restored session for user" << username;
    }
}

void AuthenticationService::onNetworkReplyFinished() {}

// --- New Remote API Helpers ---

QString AuthenticationService::redact(const QString &value) const
{
    if (value.isEmpty()) return value;
    if (value.length() <= 4) return QStringLiteral("****");
    return value.left(2) + QStringLiteral("****") + value.right(2);
}

void AuthenticationService::logRequestStart(const QString &id, const QString &endpoint, const QJsonObject &payload) const
{
    QJsonObject redacted = payload;
    if (redacted.contains("password")) redacted["password"] = "***";
    qInfo() << "AuthRequestStart" << id << endpoint << QJsonDocument(redacted).toJson(QJsonDocument::Compact);
}

void AuthenticationService::logRequestEnd(const QString &id, const QString &endpoint, int statusCode, qint64 msec, const QString &error) const
{
    qInfo() << "AuthRequestEnd" << id << endpoint << statusCode << msec << (error.isEmpty() ? "OK" : error);
}

void AuthenticationService::performApiRequest(const QString &endpoint, const QJsonObject &payload, const QString &purpose)
{
    QUrl url(m_apiBase + endpoint);
    QNetworkRequest req{url};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
    if (m_session.isValid()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ") + m_session.accessToken.toUtf8());
    }
    QJsonDocument doc(payload);
    QNetworkReply *reply = m_networkManager->post(req, doc.toJson(QJsonDocument::Compact));
    const QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    PendingRequest pr; pr.purpose = purpose; pr.endpoint = endpoint; pr.startTime = QDateTime::currentDateTimeUtc();
    m_pending.insert(reply, pr);
    logRequestStart(requestId, endpoint, payload);
    reply->setProperty("__req_id", requestId);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QString reqId = reply->property("__req_id").toString();
    auto pr = m_pending.value(reply);
    qint64 msec = pr.startTime.msecsTo(QDateTime::currentDateTimeUtc());
        QByteArray data = reply->readAll();
        QJsonObject obj;
        QJsonParseError perr;
        QJsonDocument d = QJsonDocument::fromJson(data, &perr);
        if (perr.error == QJsonParseError::NoError && d.isObject()) obj = d.object();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errorStr;
        if (reply->error() != QNetworkReply::NoError) errorStr = reply->errorString();
        logRequestEnd(reqId, pr.endpoint, status, msec, errorStr);
        if (pr.purpose == "login") {
            processLoginResponse(status, obj, errorStr);
        } else if (pr.purpose == "register") {
            processRegisterResponse(status, obj, errorStr);
        } else {
            // legacy/other
        }
        m_pending.remove(reply);
        reply->deleteLater();
    });
}

void AuthenticationService::processLoginResponse(int status, const QJsonObject &obj, const QString &networkError)
{
    if (!networkError.isEmpty()) {
        emit authenticationFinished(false, networkError);
        return;
    }
    if (status != 200) {
        QString msg = obj.value("detail").toString();
        if (msg.isEmpty()) msg = QStringLiteral("Login failed (%1)").arg(status);
        emit authenticationFinished(false, msg);
        return;
    }
    // Expected fields: access_token, token_type
    QString token = obj.value("access_token").toString();
    QString tokenType = obj.value("token_type").toString();
    if (token.isEmpty()) {
        emit authenticationFinished(false, "Missing access token");
        return;
    }
    m_session.accessToken = token;
    m_session.tokenType = tokenType.isEmpty() ? QStringLiteral("bearer") : tokenType;
    int expiresIn = obj.value("expires_in").toInt(3600);
    m_session.expiresAt = QDateTime::currentDateTimeUtc().addSecs(expiresIn);
    // Backend may return user info separately; attempt to read
    if (obj.contains("user")) {
        QJsonObject u = obj.value("user").toObject();
        m_session.userId = u.value("id").toString();
        m_session.username = u.value("username").toString();
        m_session.role = u.value("role").toString();
    }
    m_isAuthenticated = true;
    scheduleExpiryLogout(expiresIn);
    saveCredentials(m_session.accessToken, QString(), true); // always remember for now
    
    emit authenticationFinished(true, QStringLiteral("Signed in as %1").arg(m_session.username));
}

void AuthenticationService::processRegisterResponse(int status, const QJsonObject &obj, const QString &networkError)
{
    if (!networkError.isEmpty()) {
        emit registrationFinished(false, networkError);
        return;
    }
    if (status != 201 && status != 200) {
        QString msg = obj.value("detail").toString();
        if (msg.isEmpty()) msg = QStringLiteral("Registration failed (%1)").arg(status);
        emit registrationFinished(false, msg);
        return;
    }
    emit registrationFinished(true, QStringLiteral("Registration successful. Please sign in."));
}

void AuthenticationService::scheduleExpiryLogout(int secondsUntilExpiry)
{
    if (secondsUntilExpiry <= 0) {
        emit sessionExpired();
        signOut();
        return;
    }
    QTimer::singleShot(secondsUntilExpiry * 1000, this, [this]() {
        if (!m_session.isValid()) return; // already signed out
        if (QDateTime::currentDateTimeUtc() >= m_session.expiresAt) {
            emit sessionExpired();
            signOut();
        }
    });
}

void AuthenticationService::onTokenRefreshTimer()
{
    // Disabled legacy refresh; rely on expiry scheduling
}

void AuthenticationService::makeAuthRequest(const QString &endpoint, const QJsonObject &data)
{
    // Legacy path kept for backward compatibility (unused in new flow)
    performApiRequest(endpoint, data, "legacy");
}

void AuthenticationService::handleAuthResponse(const QJsonObject &response)
{
    // Legacy; new flow uses processLoginResponse
    bool success = response.value("success").toBool();
    emit authenticationFinished(success, response.value("message").toString());
}

void AuthenticationService::saveCredentials(const QString &token, const QString &refreshToken, bool rememberMe)
{
    Q_UNUSED(refreshToken)
    if (!rememberMe) return;
    if (token.isEmpty()) return;
    m_settings->setValue("auth/accessToken", token);
    m_settings->setValue("auth/expiresAt", m_session.expiresAt);
    m_settings->setValue("auth/userId", m_session.userId);
    m_settings->setValue("auth/username", m_session.username);
    m_settings->setValue("auth/role", m_session.role);
    m_settings->sync();
}

void AuthenticationService::clearCredentials()
{
    m_settings->remove("auth/accessToken");
    m_settings->remove("auth/expiresAt");
    m_settings->remove("auth/userId");
    m_settings->remove("auth/username");
    m_settings->remove("auth/role");
    m_settings->sync();
}

QString AuthenticationService::hashPassword(const QString &password, const QString &salt)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData((password + salt).toUtf8());
    return hash.result().toHex();
}

QString AuthenticationService::generateSalt()
{
    QByteArray salt;
    for (int i = 0; i < 16; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt.toHex();
}

void AuthenticationService::startTokenRefreshTimer()
{
    // Refresh token every 30 minutes (in production, this would be based on token expiry)
    m_tokenRefreshTimer->start(30 * 60 * 1000);
}

void AuthenticationService::stopTokenRefreshTimer()
{
    m_tokenRefreshTimer->stop();
}

bool AuthenticationService::authenticateLocally(const QString &phoneNumber, const QString &password)
{
    if (!m_demoUsers.contains(phoneNumber)) {
        return false;
    }
    
    QJsonObject user = m_demoUsers[phoneNumber].toObject();
    QString storedHash = user["passwordHash"].toString();
    QString salt = user["salt"].toString();
    QString providedHash = hashPassword(password, salt);
    
    return storedHash == providedHash;
}

void AuthenticationService::registerLocally(const QString &firstName, const QString &lastName,
                                          const QString &phoneNumber, const QString &password)
{
    QString salt = generateSalt();
    QString passwordHash = hashPassword(password, salt);
    
    UserProfile newUser;
    newUser.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    newUser.phoneNumber = phoneNumber;
    newUser.username = phoneNumber.right(4); // last 4 digits as simple username placeholder
    newUser.firstName = firstName;
    newUser.lastName = lastName;
    newUser.phoneVerified = false; // Would require SMS verification in real app
    newUser.twoFactorEnabled = false;
    newUser.createdAt = QDateTime::currentDateTime();
    
    saveLocalUser(newUser, passwordHash);
    
    // Add to demo users for this session
    QJsonObject userObj;
    userObj["id"] = newUser.id;
    userObj["phoneNumber"] = newUser.phoneNumber;
    userObj["username"] = newUser.username;
    userObj["firstName"] = newUser.firstName;
    userObj["lastName"] = newUser.lastName;
    userObj["passwordHash"] = passwordHash;
    userObj["salt"] = salt;
    userObj["phoneVerified"] = newUser.phoneVerified;
    userObj["twoFactorEnabled"] = newUser.twoFactorEnabled;
    userObj["createdAt"] = newUser.createdAt.toString(Qt::ISODate);
    
    m_demoUsers[phoneNumber] = userObj;
}

void AuthenticationService::saveLocalUser(const UserProfile &user, const QString &passwordHash)
{
    QSettings userSettings(QSettings::IniFormat, QSettings::UserScope, 
                          "DesktopApp", "users", this);
    
    QString userKey = QString("user_%1/").arg(user.phoneNumber);
    userSettings.setValue(userKey + "id", user.id);
    userSettings.setValue(userKey + "phoneNumber", user.phoneNumber);
    userSettings.setValue(userKey + "username", user.username);
    userSettings.setValue(userKey + "firstName", user.firstName);
    userSettings.setValue(userKey + "lastName", user.lastName);
    userSettings.setValue(userKey + "phoneVerified", user.phoneVerified);
    userSettings.setValue(userKey + "twoFactorEnabled", user.twoFactorEnabled);
    userSettings.setValue(userKey + "createdAt", user.createdAt.toString(Qt::ISODate));
    
    if (!passwordHash.isEmpty()) {
        userSettings.setValue(userKey + "passwordHash", passwordHash);
    }
    
    userSettings.sync();
}

AuthenticationService::UserProfile AuthenticationService::loadLocalUser(const QString &phoneNumber)
{
    UserProfile user;
    
    if (m_demoUsers.contains(phoneNumber)) {
        QJsonObject userObj = m_demoUsers[phoneNumber].toObject();
        user.id = userObj["id"].toString();
        user.phoneNumber = userObj["phoneNumber"].toString();
        user.username = userObj["username"].toString();
        user.firstName = userObj["firstName"].toString();
        user.lastName = userObj["lastName"].toString();
        user.phoneVerified = userObj["phoneVerified"].toBool();
        user.twoFactorEnabled = userObj["twoFactorEnabled"].toBool();
        user.createdAt = QDateTime::fromString(userObj["createdAt"].toString(), Qt::ISODate);
        user.lastLoginAt = QDateTime::currentDateTime();
    }
    
    return user;
}

} // namespace DesktopApp
