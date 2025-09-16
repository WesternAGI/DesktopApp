# DesktopApp Architecture Guide

Technical overview of DesktopApp's design, components, and system architecture.

## Overview

DesktopApp is a modular desktop chat application built with C++17 and Qt6. The architecture follows a layered design with clear separation of concerns.

### Design Principles
- **Modular:** Independent components with well-defined interfaces
- **Event-Driven:** Qt signals/slots for loose coupling
- **Extensible:** Plugin architecture for AI providers
- **Cross-Platform:** Single codebase for Windows, macOS, Linux

## Application Flow

### Startup Sequence
```
main.cpp → Application.init() → Services startup → UI initialization
```

1. **main.cpp** initializes Qt application
2. **Application** core starts all services
3. **LoginWindow** appears for authentication
4. **MainWindow** loads after successful login
5. **Event loop** begins processing user interactions

### Authentication Flow
```
LoginWindow → AuthenticationService → Validation → Session creation
```

1. User enters credentials in **LoginWindow**
2. **AuthenticationService** validates against demo accounts
3. Success creates session token and user profile
4. **MainWindow** is displayed with authenticated context

### Chat Message Flow
```
User input → MainWindow → Provider selection → AI processing → Response display
```

1. User types message in **MainWindow** input field
2. Message routed to selected **AI Provider** (EchoProvider for demo)
3. Provider processes request and generates response
4. Response displayed in conversation area

## Project Structure

```
DesktopApp/
├── src/                    # Source code
│   ├── core/              # Application core
│   ├── ui/                # User interface
│   ├── services/          # Business logic
│   ├── providers/         # AI integrations
│   ├── theme/             # Visual design
│   └── data/              # Data models
├── docs/                  # Documentation
├── icons/                 # Application icons
├── scripts/               # Build/setup scripts
└── CMakeLists.txt         # Build configuration
```

## Components

### Application Core (`src/core/`)

**Application.h/.cpp**
- Main application coordinator and service container
- Initializes all services in correct order
- Manages application lifecycle and cleanup
- Provides service discovery and dependency injection

**main.cpp**
- Program entry point and Qt application setup
- Handles authentication flow and window management
- Supports `--skip-auth` flag for development mode
- Event loop initialization and exception handling

### User Interface (`src/ui/`)

**LoginWindow.h/.cpp**
- Authentication interface with login/registration forms
- Handles username/password input and validation
- Connects to AuthenticationService for credential verification
- Demo authentication for development

**MainWindow.h/.cpp**
- Primary chat interface with conversation management
- Left panel: conversation list and navigation
- Center area: message display and chat history
- Bottom section: message input and send controls
- Menu bar: settings, themes, and application controls

### Services Layer (`src/services/`)

**AuthenticationService.h/.cpp**
- User authentication and session management
- Demo account verification
- Session token management and automatic refresh
- Password hashing and security utilities

**SettingsStore.h/.cpp**
- Application configuration persistence
- User preferences (theme, window size, etc.)
- Cross-platform settings storage using Qt
- Settings validation and migration

**AudioRecorder.h/.cpp**
- Voice message recording capabilities
- Audio format conversion and compression
- Microphone access and permission handling

### AI Providers (`src/providers/`)

**Provider.h**
- Abstract base class for all AI providers
- Defines standard interface: sendMessage(), getResponse()
- Plugin architecture for adding new AI services

**EchoProvider.h/.cpp**
- Demo provider that echoes user messages
- Simulates AI response timing and behavior
- Used for testing and development

**Future Providers** (planned)
- **ChatGPTProvider:** OpenAI GPT integration
- **ClaudeProvider:** Anthropic Claude integration
- **LocalProvider:** Local AI model support

### Theme System (`src/theme/`)

**ThemeManager.h/.cpp**
- Light/dark theme switching
- CSS stylesheet management
- Theme persistence in user settings
- Dynamic theme updates without restart

**IconRegistry.h/.cpp**
- SVG icon loading and caching
- Theme-aware icon variations
- Icon scaling for different DPI settings

### Data Models (`src/data/`)

**Models.h**
- Core data structures: User, Conversation, Message
- JSON serialization/deserialization
- Data validation and type safety

**ConversationStore.h/.cpp**
- Local storage for conversation history
- Message persistence and retrieval
- JSON-based file storage

## Architectural Patterns

### Qt Signals/Slots
- **Loose Coupling:** Components communicate via signals without direct references
- **Event-Driven:** UI updates automatically respond to data changes
- **Thread Safety:** Cross-thread communication handled by Qt

Example:
```cpp
// AuthenticationService emits signal on successful login
emit authenticationFinished(true, "Login successful");

// LoginWindow connects to handle the result
connect(authService, &AuthenticationService::authenticationFinished,
        this, &LoginWindow::onAuthenticationFinished);
```

### Service Locator Pattern
- **Application** class provides centralized service access
- Services registered at startup and available globally
- Facilitates dependency injection and testing

### Provider Pattern
- **Abstract Provider** interface for AI services
- Runtime provider selection and switching
- Easy addition of new AI providers without core changes

## Build System

### CMake Configuration
- **CMakeLists.txt:** Main build configuration
- **Cross-platform:** Handles Qt finding and linking
- **Targets:** Separate library (DesktopAppLib) and executable (DesktopApp)
- **Dependencies:** Automatic Qt component detection

### Library Structure
- **DesktopAppLib:** Static library containing all core functionality
- **DesktopApp:** Minimal executable that links to library
- **Benefits:** Faster incremental builds, easier testing

## Development Guidelines

### Code Organization
- **Headers (.h):** Class declarations and interfaces
- **Implementation (.cpp):** Method definitions and logic
- **Qt MOC:** Automatic signal/slot code generation
- **Naming:** Qt conventions (camelCase, m_ member prefix)

### Adding New Features

1. **Define Interface:** Create header file with class declaration
2. **Implement Logic:** Add corresponding .cpp file
3. **Update CMake:** Add new files to CMakeLists.txt
4. **Connect Services:** Register with Application if needed
5. **Add Tests:** Create unit tests for new functionality

### Debugging and Logging
- **qDebug():** Qt logging for development output
- **Categories:** Use logging categories for filtering
- **Release Builds:** Logging automatically disabled

## Performance Considerations

### Memory Management
- **Qt Parent-Child:** Automatic memory cleanup
- **RAII:** Resource acquisition in constructors
- **Smart Pointers:** For complex ownership scenarios

### Threading
- **Main Thread:** UI and event processing
- **Worker Threads:** Network requests and file I/O
- **Qt::QueuedConnection:** Safe cross-thread signals

### Optimization
- **Lazy Loading:** Load conversations and messages on demand
- **Icon Caching:** SVG icons rendered once and cached
- **Theme Updates:** Minimal repainting on theme changes

## Data Models

### Core Structures
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

### Storage Strategy
- **JSON-based:** Simple, portable, human-readable
- **Local Files:** No database dependency
- **Backup Friendly:** Easy to backup and restore

## Dependencies

### Required
- **Qt6:** Core, Widgets, Network, Multimedia
- **C++17:** Modern C++ features and standard library
- **CMake 3.21+:** Build system

### Platform Support
- **Windows:** Windows 10 and later
- **macOS:** macOS 10.15 and later
- **Linux:** Modern distributions with Qt6 support

## Future Enhancements

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

---

This architecture is designed to be simple to understand but flexible enough to grow into a full-featured application. The modular design allows working on individual components without affecting others.
