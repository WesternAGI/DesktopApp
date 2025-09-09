# DesktopApp Security Guide

Security features and considerations for DesktopApp.

## Security Status: Basic Protection

**Current Level:** Prototype security for local development  
**Risk Level:** Low (local-only application, no network features)

## Current Security Features

### ✅ Implemented
- **Local Storage:** All data stays on your computer
- **Demo Authentication:** Basic password hashing for test accounts
- **Session Management:** Secure session tokens instead of password storage
- **File Permissions:** Standard OS file protection
- **No Network Risk:** No cloud sync or remote data transmission

### ⚠️ Planned (Not Yet Implemented)
- Message encryption for data at rest
- Secure file attachment handling
- Two-factor authentication (2FA)
- OAuth integration with external services
- Advanced password requirements

## Data Storage

### Local Files
**Windows:** `%APPDATA%\DesktopApp Project\DesktopApp\`  
**macOS:** `~/Library/Preferences/DesktopApp Project/DesktopApp/`  
**Linux:** `~/.config/DesktopApp Project/DesktopApp/`

### What's Stored
- Application settings and preferences
- Demo account session tokens
- Theme and UI preferences
- No sensitive user data or messages (in prototype)

## Privacy

### Data Collection
- **None:** No analytics, telemetry, or usage tracking
- **Local Only:** All data remains on your device
- **No Network:** No data transmitted to external servers

### Future Privacy Features
- Optional message encryption
- Local-only AI processing options
- Data export and deletion tools
- Privacy-focused AI provider options

## Security Best Practices

### For Users
- Keep your system updated with latest security patches
- Use strong passwords when real authentication is implemented
- Review file permissions in data directories
- Be aware this is a prototype with limited security features

### For Developers
- Follow secure coding practices
- Validate all user inputs
- Use Qt's secure storage mechanisms
- Plan for security from the beginning

## Reporting Security Issues

This is a prototype project. For security concerns:
1. Check if the issue affects the current local-only scope
2. Consider the planned network features in your assessment
3. Report issues through standard project channels
4. Include impact assessment and suggested mitigations

## Disclaimer

DesktopApp is currently a prototype application focused on local development and testing. Security features are basic and suitable only for non-production use. Do not use for sensitive or confidential communications.
- **Linux**: `~/.config/DesktopApp Project/DesktopApp/`

**Future Storage Security:**
- Message encryption for sensitive conversations
- Secure file vault for attachments
- Optional master password for extra protection
- Automatic data cleanup options

### 3. Network Security

**Current Status:**
- No network communication in prototype
- No external API calls
- Local-only operation

**Future Network Features:**
- TLS encryption for all external communication
- Certificate pinning for AI provider APIs
- Secure credential storage for API keys
- Request signing and validation

### 4. Privacy Protection

**Current Privacy Features:**
- No telemetry or analytics collection
- No data sharing with third parties
- Local-only processing
- No online account requirements

**Privacy Principles:**
- User data ownership
- Transparent data handling
- Minimal data collection
- User control over data

## Security Best Practices

### For Users

**Keeping Your Data Safe:**
1. **Regular Backups**: Export your conversations regularly
2. **Strong Passwords**: Use strong passwords for your system account
3. **System Updates**: Keep your operating system updated
4. **Antivirus**: Use reliable antivirus software
5. **Physical Security**: Lock your computer when away

**Privacy Tips:**
1. **Demo Accounts**: Remember that demo accounts are for testing only
2. **Sensitive Data**: Don't store highly sensitive information in the prototype
3. **File Sharing**: Be careful about what files you might attach in future versions
4. **Screen Sharing**: Be aware of who can see your screen during chats

### For Developers

**Secure Development Practices:**
1. **Input Validation**: Always validate user input
2. **Error Handling**: Don't expose sensitive information in error messages
3. **Dependencies**: Keep Qt and other dependencies updated
4. **Code Review**: Review security-sensitive code changes
5. **Testing**: Test security features thoroughly

**Adding Security Features:**
1. **Encryption**: Use established libraries (Qt's crypto, OpenSSL)
2. **Authentication**: Follow industry best practices
3. **Storage**: Use Qt's secure storage APIs
4. **Network**: Implement proper TLS validation

## Known Security Limitations

### Current Prototype Limitations

❌ **No Message Encryption**
- Conversations stored in plain text JSON files
- No protection if computer is compromised
- No secure deletion of sensitive messages

❌ **Basic Authentication**
- Demo accounts only
- Simple password hashing
- No protection against advanced attacks

❌ **No File Security**
- Future file attachments will need security validation
- No virus scanning integration
- No secure file storage

❌ **Limited Access Control**
- Single user per installation
- No permission system
- No audit logging

### Mitigation Strategies

**For Current Use:**
1. Use only for testing and development
2. Don't store sensitive real conversations
3. Keep your system secure with standard practices
4. Regularly clear demo data if needed

**For Future Deployment:**
1. Implement proper encryption before production use
2. Add comprehensive authentication
3. Include security auditing and logging
4. Regular security assessments

## Planned Security Roadmap

### Phase 1: Basic Security (Next Release)
- ✅ Secure password hashing with salt
- ✅ Encrypted local storage for messages
- ✅ Secure credential storage for API keys
- ✅ Input validation and sanitization

### Phase 2: Enhanced Security
- 🔄 Two-factor authentication
- 🔄 Message encryption with user keys
- 🔄 Secure file attachment handling
- 🔄 Audit logging and monitoring

### Phase 3: Advanced Security
- 📋 Enterprise features (SSO, policies)
- 📋 Advanced threat protection
- 📋 Security compliance features
- 📋 Professional security auditing

## Security Configuration

### Current Settings

**Available Security Options:**
- Login session timeout (basic)
- Theme selection (affects screen privacy)
- Data location preferences

### Future Security Settings

**Planned Security Controls:**
- Message encryption on/off
- Auto-logout timer
- Data retention policies
- Privacy mode options
- Secure deletion settings

## Reporting Security Issues

### If You Find a Security Problem

**For Security Bugs:**
1. **Don't post publicly** - security issues should be reported privately
2. **Include details**: What you found, how to reproduce it
3. **Provide context**: What version, what platform
4. **Be patient**: Security fixes take time to implement properly

**What We'll Do:**
1. Acknowledge receipt of your report
2. Investigate the issue
3. Develop and test a fix
4. Release security updates
5. Credit you for responsible disclosure (if desired)

### Security Update Process

**How Security Updates Work:**
1. Critical security fixes get priority
2. Updates include clear security advisories
3. Users are notified of security updates
4. Automatic update checking (planned feature)

## Security Resources

### For Users
- Keep your operating system updated
- Use reputable antivirus software
- Learn about phishing and social engineering
- Use strong, unique passwords

### For Developers
- [Qt Security Guide](https://doc.qt.io/qt-6/security.html)
- [OWASP Security Guidelines](https://owasp.org/)
- [Secure Coding Practices](https://wiki.sei.cmu.edu/confluence/display/seccode)
- [C++ Security Best Practices](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-security)

---

**Remember**: This is a prototype application. The security features are basic and should not be relied upon for protecting sensitive data. Always follow general computer security best practices.

## Data Handling (Current Minimal Behavior)
* All data lives in the user profile application data directory (see `Application::appDataDir`).
* JSON files are plain text; delete them to reset the app.
* No background transmission of conversation content.

### Local Storage Hardening (Planned)
| Item | Planned Approach |
|------|------------------|
| Encryption | AES‑256 GCM (libsodium) with passphrase-derived key |
| Key derivation | Argon2id with per-file or per-vault salt |
| Integrity | Authenticated encryption tags |
| Secret rotation | Re-encrypt on passphrase change |

#### Data Files
DesktopApp stores conversation data as JSON files under the application data directory:

```
<AppData>/DesktopApp/
    conversations.json
    messages.json
```

Best practices:
- Store data in the user-scoped app data directory
- Use debounced autosave to reduce write amplification
- Validate and sanitize content before persistence

#### Optional Encrypted Exports
For users who need encryption at rest, provide an option to export/import data with encryption (future enhancement). Recommended approach:
- AES-256-GCM with random IV per file
- Keys derived via PBKDF2/Argon2id with per-export salt
- Authenticate payloads to prevent tampering

#### File Encryption
```cpp
class FileVault {
private:
    QByteArray encryptFile(const QByteArray& data, const QString& key) {
        QAESEncryption encryption(QAESEncryption::AES_256, 
                                 QAESEncryption::CBC);
        
        // Generate random IV
        QByteArray iv = generateRandomBytes(16);
        
        // Encrypt data
        QByteArray encrypted = encryption.encode(data, key.toUtf8(), iv);
        
        // Prepend IV to encrypted data
        return iv + encrypted;
    }
};
```

**Protection Levels**:
- **Sensitive Files**: API keys, credentials, personal documents
- **Standard Files**: Images, regular attachments
- **Public Files**: Non-sensitive data, temporary files

## Credential / Session Handling
Current session token is a random UUID stored in settings; purely illustrative. Replace with secure opaque tokens or avoid entirely for local-only mode.

#### Platform Integration
```cpp
class SettingsStore {
private:
    void storeSecureValue(const QString& key, const QString& value) {
#ifdef Q_OS_WIN
        // Windows Credential Manager
        storeCredentialWindows(key, value);
#elif defined(Q_OS_MAC)
        // macOS Keychain
        storeCredentialMacOS(key, value);
#else
        // Linux: libsecret or encrypted file
        storeCredentialLinux(key, value);
#endif
    }
};
```

**Security Features**:
- **Platform Keychain**: Native secure storage on each platform
- **Access Control**: Application-specific credential storage
- **Encryption**: Additional encryption layer for stored credentials
- **Automatic Cleanup**: Secure deletion when uninstalling

#### Credential Types
1. **API Keys**: AI provider authentication tokens
2. **Passwords**: Export encryption passwords (when used)
3. **Certificates**: Client certificates for authentication
4. **Tokens**: OAuth and session tokens

## Memory
No secure string abstractions implemented. Consider introducing a small RAII wrapper only where truly needed (avoid premature complexity).

#### Secure String Handling
```cpp
class SecureString {
private:
    std::vector<char> m_data;
    
public:
    SecureString(const QString& data) {
        // Copy to secure memory
        m_data.resize(data.size());
        std::memcpy(m_data.data(), data.toUtf8().data(), data.size());
        
        // Lock memory pages (prevent swapping)
        mlock(m_data.data(), m_data.size());
    }
    
    ~SecureString() {
        // Secure wipe
        explicit_bzero(m_data.data(), m_data.size());
        munlock(m_data.data(), m_data.size());
    }
};
```

**Memory Protection**:
- **Secure Allocation**: Locked memory pages for sensitive data
- **Overwrite on Free**: Explicit zeroing of sensitive memory
- **Stack Protection**: Guard pages and canaries
- **ASLR**: Address space layout randomization

## Network
No network traffic today. When adding providers: enforce TLS 1.2+, validate cert chains, optionally pin endpoints, sanitize request/response JSON.

### TLS/SSL Configuration
```cpp
class NetworkManager {
private:
    void configureTLS(QNetworkAccessManager* manager) {
        // Enforce TLS 1.2+
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setProtocol(QSsl::TlsV1_2OrLater);
        
        // Strict certificate validation
        config.setPeerVerifyMode(QSslSocket::VerifyPeer);
        
        // Strong cipher suites only
        config.setCiphers(getSecureCiphers());
        
        QSslConfiguration::setDefaultConfiguration(config);
    }
};
```

**Security Measures**:
- **TLS 1.2+**: Minimum supported TLS version
- **Certificate Pinning**: Pin certificates for critical services
- **OCSP Stapling**: Online certificate status validation
- **HSTS**: HTTP Strict Transport Security headers

### API Security
```cpp
class AIProvider {
protected:
    QNetworkRequest createSecureRequest(const QUrl& url) {
        QNetworkRequest request(url);
        
        // Security headers
        request.setRawHeader("User-Agent", getUserAgent());
        request.setRawHeader("X-Request-ID", generateRequestId());
        
        // Authentication
        QString signature = generateHMAC(request.url().toString());
        request.setRawHeader("Authorization", signature.toUtf8());
        
        return request;
    }
};
```

**Authentication Methods**:
- **API Keys**: Secure storage and transmission
- **OAuth 2.0**: Token-based authentication
- **HMAC Signatures**: Request signing for integrity
- **Rate Limiting**: Prevent abuse and enumeration

## Input Validation
UI inputs do very light validation (email format, password length). No HTML/Markdown sanitization layer yet—introduce if rendering untrusted rich text.

### User Input Processing
```cpp
class MessageValidator {
public:
    ValidationResult validateMessage(const QString& message) {
        ValidationResult result;
        
        // Length validation
        if (message.length() > MAX_MESSAGE_LENGTH) {
            result.addError("Message too long");
        }
        
        // Content validation
        if (containsUnsafeContent(message)) {
            result.addError("Unsafe content detected");
        }
        
        // Encoding validation
        if (!isValidUTF8(message)) {
            result.addError("Invalid character encoding");
        }
        
        return result;
    }
    
private:
    bool containsUnsafeContent(const QString& message) {
        // Check for potential XSS, injection attacks
        static const QRegularExpression unsafePatterns(
            R"(<script|javascript:|data:|vbscript:)",
            QRegularExpression::CaseInsensitiveOption
        );
        
        return unsafePatterns.match(message).hasMatch();
    }
};
```

### File Upload Security
```cpp
class FileValidator {
public:
    bool validateFile(const QString& filePath) {
        QFileInfo fileInfo(filePath);
        
        // Size validation
        if (fileInfo.size() > MAX_FILE_SIZE) {
            return false;
        }
        
        // Type validation
        QString mimeType = detectMimeType(filePath);
        if (!isAllowedMimeType(mimeType)) {
            return false;
        }
        
        // Content validation
        if (!scanFileContent(filePath)) {
            return false;
        }
        
        return true;
    }
    
private:
    bool scanFileContent(const QString& filePath) {
        // Basic malware scanning
        // Integration with ClamAV or Windows Defender
        return performVirusScan(filePath);
    }
};
```

**Validation Layers**:
1. **Client-Side**: Immediate feedback and basic validation
2. **Server-Side**: Comprehensive validation and sanitization
3. **Storage**: Final validation before persistence
4. **Display**: Output encoding and escaping

## Authentication
Local demo user map in memory + simple salted hash comparison. Not suitable for production.

### User Authentication
```cpp
class AuthenticationManager {
public:
    bool authenticateUser(const QString& password) {
        // Get stored password hash
        QString storedHash = getStoredPasswordHash();
        
        // Verify password using secure comparison
        return verifyPassword(password, storedHash);
    }
    
private:
    QString hashPassword(const QString& password, const QString& salt) {
        // Use Argon2id for password hashing
        return Argon2id::hash(password, salt, MEMORY_COST, TIME_COST);
    }
    
    bool verifyPassword(const QString& password, const QString& hash) {
        return Argon2id::verify(password, hash);
    }
};
```

**Authentication Features**:
- **Strong Hashing**: Argon2id for password storage
- **Salt Generation**: Random salt for each password
- **Timing Attack Protection**: Constant-time comparison
- **Account Lockout**: Protection against brute force attacks

### Permission System
```cpp
enum class Permission {
    ReadConversations,
    WriteConversations,
    DeleteConversations,
    AccessSettings,
    ManageProviders,
    ExportData
};

class PermissionManager {
public:
    bool hasPermission(Permission permission) {
        return m_userPermissions.contains(permission);
    }
    
    void grantPermission(Permission permission) {
        m_userPermissions.insert(permission);
        emit permissionChanged(permission, true);
    }
};
```

## Privacy
Entirely local; nothing leaves machine unless a future provider sends text to an API (not implemented yet). Keep it that way by design unless user opts in.

### Data Minimization
- **Selective Logging**: Only log necessary information
- **Automatic Cleanup**: Periodic deletion of temporary data
- **Anonymization**: Remove PII from analytics data
- **User Control**: Settings to control data retention

### Analytics Privacy
```cpp
class AnalyticsManager {
public:
    void sendAnonymousEvent(const QString& event) {
        if (!isAnalyticsEnabled()) return;
        
        QJsonObject payload;
        payload["event"] = event;
        payload["timestamp"] = QDateTime::currentSecsSinceEpoch();
        payload["session_id"] = getAnonymousSessionId();
        
        // No personal information included
        sendAnalytics(payload);
    }
    
private:
    QString getAnonymousSessionId() {
        // Generate ephemeral session ID
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
};
```

**Privacy Measures**:
- **Opt-In Analytics**: User must explicitly enable analytics
- **No PII**: Personal information never sent
- **Local Processing**: Analysis done locally when possible
- **Data Retention**: Automatic deletion of old analytics data

## Recommended Next Security Steps
1. Add LICENSE and SECURITY policy clarifying prototype status.
2. Introduce a minimal threat model doc once real networking appears.
3. Enable static analysis (clang-tidy, cppcheck) in CI.
4. Add unit tests for auth, file IO, and JSON parsing robustness.
5. Provide a safe migration path before introducing encryption (avoid locking out early adopters).

---
This document is intentionally honest: security is minimal right now. Treat current code as a starting point, not a hardened application.

### Code Security
```cpp
// Example of secure coding practices

// 1. Bounds checking
bool safeArrayAccess(const QVector<Message>& messages, int index) {
    if (index < 0 || index >= messages.size()) {
        qWarning() << "Array bounds violation prevented";
        return false;
    }
    return true;
}

// 2. Integer overflow protection
bool safeAdd(int a, int b, int& result) {
    if (a > 0 && b > INT_MAX - a) return false;
    if (a < 0 && b < INT_MIN - a) return false;
    result = a + b;
    return true;
}

// 3. Resource management
class SecureResource {
public:
    SecureResource() : m_resource(allocateResource()) {}
    ~SecureResource() { secureCleanup(); }
    
    // Prevent copying
    SecureResource(const SecureResource&) = delete;
    SecureResource& operator=(const SecureResource&) = delete;
    
private:
    void secureCleanup() {
        if (m_resource) {
            // Secure wipe before deallocation
            explicit_bzero(m_resource, m_resourceSize);
            deallocateResource(m_resource);
        }
    }
};
```

### Build Security
```cmake
# CMakeLists.txt security flags

# Enable security features
if(MSVC)
    # Stack protection
    target_compile_options(${PROJECT_NAME} PRIVATE /GS)
    # Control Flow Guard
    target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
    # Address Space Layout Randomization
    target_link_options(${PROJECT_NAME} PRIVATE /DYNAMICBASE)
else()
    # Stack protection
    target_compile_options(${PROJECT_NAME} PRIVATE -fstack-protector-strong)
    # Position Independent Executable
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIE)
    # Fortify source
    target_compile_definitions(${PROJECT_NAME} PRIVATE _FORTIFY_SOURCE=2)
endif()
```

## Incident Response

### Security Event Logging
```cpp
class SecurityLogger {
public:
    enum class EventType {
        AuthenticationFailure,
        UnauthorizedAccess,
        DataAccess,
        ConfigurationChange,
        NetworkError
    };
    
    void logSecurityEvent(EventType type, const QString& details) {
        QJsonObject event;
        event["type"] = static_cast<int>(type);
        event["timestamp"] = QDateTime::currentSecsSinceEpoch();
        event["details"] = details;
        event["source"] = QCoreApplication::applicationName();
        
        writeSecurityLog(event);
    }
};
```

### Breach Response
1. **Detection**: Automated monitoring and alerts
2. **Containment**: Immediate isolation of affected systems
3. **Assessment**: Determine scope and impact
4. **Notification**: Inform users if necessary
5. **Recovery**: Restore secure operations
6. **Lessons Learned**: Update security measures

## Security Checklist

### For Users
- [ ] Use strong password for encrypted exports/backups
- [ ] Enable automatic updates
- [ ] Regularly backup encrypted data
- [ ] Review privacy settings
- [ ] Monitor for suspicious activity
- [ ] Use updated operating system
- [ ] Keep antivirus software current

### For Developers
- [ ] Regular security audits
- [ ] Dependency vulnerability scanning
- [ ] Static code analysis
- [ ] Penetration testing
- [ ] Secure coding training
- [ ] Code review process
- [ ] Incident response plan

### For System Administrators
- [ ] Network security monitoring
- [ ] Access control policies
- [ ] Regular security updates
- [ ] Backup and recovery testing
- [ ] User security training
- [ ] Compliance verification
- [ ] Threat intelligence monitoring

## Compliance & Standards

### Standards Compliance
- **OWASP Top 10**: Protection against common vulnerabilities
- **ISO 27001**: Information security management
- **NIST Framework**: Cybersecurity best practices
- **GDPR Principles**: Privacy by design and default

### Security Certifications
- **Common Criteria**: Security evaluation criteria
- **FIPS 140-2**: Cryptographic module validation
- **SOC 2**: Security and availability controls

## Conclusion

Security is an ongoing process that requires constant vigilance and improvement. This guide provides the foundation for maintaining security throughout the development and deployment lifecycle of DesktopApp.

For security issues or questions, please contact: security@desktopapp.com

---

**Remember: Security is everyone's responsibility.**
