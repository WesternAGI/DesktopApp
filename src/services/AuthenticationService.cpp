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
{
    connect(m_tokenRefreshTimer, &QTimer::timeout, this, &AuthenticationService::onTokenRefreshTimer);
    
    // Try to restore previous session
    restoreSession();
}AuthenticationService::~AuthenticationService() = default;

void AuthenticationService::signIn(const QString &usernameOrPhone, const QString &password, bool rememberMe)
{
    Q_UNUSED(rememberMe)
    qDebug() << "AuthenticationService: Sign in for" << usernameOrPhone;
    QJsonObject payload;
    payload["username"] = usernameOrPhone;
    payload["password"] = password;
    performApiRequest("/token", payload, "login");
}

void AuthenticationService::registerUser(const QString &username, const QString &phoneNumber, const QString &password)
{
    qDebug() << "AuthenticationService: Registration for" << username;
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
    
    QJsonObject requestData;
    requestData["phoneNumber"] = phoneNumber;
    
    makeAuthRequest("/auth/reset-password", requestData);
}

void AuthenticationService::verifyTwoFactor(const QString &code)
{
    qDebug() << "AuthenticationService: Verifying 2FA code";
    
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

} // namespace DesktopApp
