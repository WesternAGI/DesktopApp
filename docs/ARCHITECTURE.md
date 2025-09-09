# GadAI Architecture Guide

Technical overview of GadAI's design and structure.

## Overview

GadAI is a C++17/Qt6 desktop chat application with modular architecture for easy extension and maintenance.

### Design Principles
- **Modular:** Separate components for different responsibilities
- **Simple:** Clear code organization and minimal complexity
- **Extensible:** Easy to add new features and AI providers
- **Cross-Platform:** Works on Windows, macOS, and Linux

## Project Structure

```
src/
├── core/           # Application startup and coordination
├── ui/             # User interface components
├── services/       # Business logic and data management
├── providers/      # AI provider integrations
├── theme/          # Theming and visual design
└── data/           # Data models and storage
```

## Core Components

### Application Core
- **Application.h/cpp:** Main coordinator, service startup, lifecycle management
- **main.cpp:** Entry point, authentication flow, window management

### User Interface
- **LoginWindow:** Authentication interface with username/password forms
- **MainWindow:** Main chat interface with conversation list and message area
- **Dialogs:** Settings, about, and other modal dialogs

### Services
- **AuthenticationService:** Login, session management, demo accounts
- **SettingsStore:** Application preferences and configuration
- **AudioRecording:** Voice message recording (planned)

### Providers
- **EchoProvider:** Demo AI provider for testing
- **Future:** ChatGPT, Claude, custom AI integrations

### Theme System
- **ThemeManager:** Light/dark theme switching
- **IconRegistry:** Icon loading and management

## Key Architectural Patterns

### Qt Signals/Slots
- Event-driven communication between components
- Loose coupling between UI and business logic
- Thread-safe asynchronous operations

### Service Layer
- Business logic separated from UI
- Dependency injection for testing
- Clear service interfaces

### Provider Pattern
- Pluggable AI provider architecture
- Standard interface for different AI services
- Easy to add new providers

## Build System

### CMake Configuration
- Cross-platform build support
- Qt6 integration and dependency management
- Separate library and executable targets

### Dependencies
- **Qt6:** Core, Widgets, Multimedia, Network
- **C++17:** Modern C++ features and standards
- **Platform:** Windows (MSVC/MinGW), macOS (Clang), Linux (GCC)

## Development Guidelines

### Code Organization
- Header files define interfaces
- Implementation in corresponding .cpp files
- Qt MOC system for signals/slots
- RAII for resource management

### Adding Features
1. Define service interface in `services/`
2. Implement UI components in `ui/`
3. Update CMakeLists.txt for new files
4. Follow Qt naming conventions and patterns

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
