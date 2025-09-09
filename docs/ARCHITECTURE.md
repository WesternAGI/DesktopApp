# GadAI Architecture Guide

This document explains how the GadAI desktop chat application is designed and built.

## Overview

GadAI is a desktop chat application built with C++17 and Qt6. It uses a clean, modular architecture that separates concerns and makes the code easy to understand and extend.

### Key Design Principles

1. **Simple and Clear**: Code is organized in a way that's easy to follow
2. **Modular**: Different parts of the app are separated into logical components
3. **Extensible**: New features can be added without breaking existing code
4. **Cross-Platform**: Works on Windows, macOS, and Linux

## Project Structure

```
src/
├── core/               # Application startup and core services
│   ├── Application.h   # Main application coordinator
│   └── main.cpp        # Program entry point
├── ui/                 # User interface components
│   ├── LoginWindow.h   # Login screen
│   ├── MainWindow.h    # Main chat window
│   └── ...             # Other UI components
├── services/           # Business logic and data services
│   ├── AuthenticationService.h  # Login and user management
│   ├── SettingsStore.h          # App settings and preferences
│   └── ...                      # Other services
├── providers/          # AI provider integrations
│   ├── EchoProvider.h  # Demo AI provider for testing
│   └── ...             # Future AI providers (ChatGPT, Claude, etc.)
├── theme/              # Theming and visual design
│   ├── ThemeManager.h  # Light/dark theme switching
│   └── IconRegistry.h  # Icon management
└── data/               # Data models and storage
    └── Models.h        # Conversation, Message, User models
```

## Core Components

### 1. Application Core

**Application.h/cpp** - The main coordinator
- Starts up all the services
- Manages the application lifecycle
- Coordinates between different parts of the app
- Handles shutdown and cleanup

**main.cpp** - Program entry point
- Creates the Qt application
- Shows the login window
- Launches the main window after successful login

### 2. User Interface Layer

**LoginWindow** - Authentication interface
- Simple phone number + password form
- Demo account support for testing
- Session management integration

**MainWindow** - Main chat interface
- Three-panel layout (conversations, messages, input)
- Menu system and keyboard shortcuts
- Theme switching and settings access

**UI Design Philosophy:**
- Clean, modern interface similar to popular chat apps
- Responsive layout that works on different screen sizes
- Consistent styling using Qt's theme system

### 3. Services Layer

Services handle the business logic and data management:

**AuthenticationService**
- Manages user login/logout
- Handles demo accounts
- Session persistence
- Remote authentication (for future real backends)

**SettingsStore**
- App preferences and configuration
- Theme settings
- User preferences
- Cross-platform settings storage using Qt

**Other Services** (basic implementations)
- AudioRecorder: Voice message recording
- SearchEngine: Find messages in conversations
- FileVault: Secure file storage

### 4. AI Provider System

**Provider Architecture:**
- Abstract interface that all AI providers implement
- Pluggable design - easy to add new AI services
- Consistent API regardless of which AI service is used

**EchoProvider** (demo implementation)
- Responds with test messages
- Used for development and testing
- Shows how real providers would work

**Future Providers** (not implemented yet)
- OpenAI/ChatGPT integration
- Anthropic Claude integration
- Local AI model support

### 5. Theme System

**ThemeManager**
- Light and dark theme support
- System theme detection
- Consistent styling across all windows
- Icon color adaptation

**IconRegistry**
- SVG icon management
- Theme-aware icon coloring
- Efficient icon caching

### 6. Data Layer

**Models.h** - Data structures
```cpp
struct Conversation {
    QString id;
    QString title;
    QDateTime lastActivity;
    QList<Message> messages;
};

struct Message {
    QString id;
    QString content;
    QString sender;
    QDateTime timestamp;
    MessageType type;
};
```

**Storage** (basic implementation)
- JSON-based local storage
- No database dependency (keeps things simple)
- Easy backup and portability

## Key Design Patterns

### 1. Service Locator Pattern
The Application class acts as a central registry for all services:

```cpp
class Application {
public:
    AuthenticationService* auth() const;
    SettingsStore* settings() const;
    ThemeManager* theme() const;
    // ... other services
};
```

### 2. Signal-Slot Communication
Uses Qt's built-in signal-slot system for communication between components:

```cpp
// Authentication service signals login success
connect(authService, &AuthenticationService::loginSuccessful,
        this, &MainWindow::onLoginSuccess);
```

### 3. Provider Interface Pattern
All AI providers implement the same interface:

```cpp
class AIProvider {
public:
    virtual void sendMessage(const QString& message) = 0;
    virtual bool isAvailable() const = 0;
    // ... other common methods
};
```

## Data Flow

### 1. Application Startup
1. `main.cpp` creates Qt application
2. Application class initializes all services
3. LoginWindow appears
4. User authenticates with demo account
5. MainWindow launches and connects to services

### 2. Sending a Message
1. User types in message input field
2. MainWindow receives text and calls AI provider
3. Provider processes message (or echoes for demo)
4. Response comes back via signals
5. UI updates to show the conversation

### 3. Theme Switching
1. User clicks theme toggle or changes setting
2. ThemeManager updates all stylesheets
3. IconRegistry reloads icons with new colors
4. All windows refresh their appearance

## Security Considerations

### Current Security Features
- Settings stored securely using Qt's platform-specific storage
- No plaintext password storage
- Local-only data (no cloud sync risks)

### Future Security (not implemented yet)
- Message encryption
- Secure credential storage
- File attachment security
- Privacy controls

## Performance Design

### Efficient Patterns Used
- Lazy loading of conversations
- Efficient Qt data structures
- Minimal memory copying
- Smart pointer usage for automatic memory management

### Scalability Considerations
- Services can be easily swapped or upgraded
- Provider system allows multiple AI backends
- Theme system supports unlimited themes
- Data models designed for large conversation histories

## Testing Strategy

### Current Testing (basic)
- Demo providers for isolated testing
- Manual testing of UI components
- Build verification on multiple platforms

### Future Testing (planned)
- Unit tests for all services
- Integration tests for AI providers
- UI automation testing
- Performance benchmarks

## Extension Points

### Adding New AI Providers
1. Implement the AIProvider interface
2. Add provider-specific configuration
3. Register with the provider manager
4. Test with the existing UI

### Adding New Features
1. Create service classes for business logic
2. Add UI components if needed
3. Wire together with signals/slots
4. Update settings if configurable

### Customizing Themes
1. Add new stylesheets to theme manager
2. Create corresponding icon sets
3. Register theme in the theme system
4. Test across all UI components

## Build System

### CMake Configuration
- Cross-platform build support
- Automatic Qt6 detection
- Proper dependency management
- Debug and release configurations

### Dependencies
- **Qt6**: Core, Widgets, Network, Multimedia
- **C++17**: Modern C++ features and standard library
- **CMake 3.21+**: Build system
- **Platform**: Windows, macOS, Linux support

## Deployment

### Current Deployment
- Local build and run
- Development-friendly setup
- All dependencies bundled with Qt

### Future Deployment (planned)
- Installer packages for each platform
- Automatic updates
- Code signing for security
- Distribution through package managers

## Future Architecture Plans

### Short Term
- Complete AI provider implementations
- Improve data persistence
- Add comprehensive error handling
- Implement proper logging

### Medium Term
- Plugin architecture for extensions
- Cloud synchronization support
- Advanced security features
- Performance optimizations

### Long Term
- Multi-platform mobile apps
- Web interface
- Enterprise features
- Advanced AI integrations

---

This architecture is designed to be simple enough to understand quickly, but flexible enough to grow into a full-featured application. The modular design means you can work on one part without affecting others, and new developers can quickly understand how everything fits together.
