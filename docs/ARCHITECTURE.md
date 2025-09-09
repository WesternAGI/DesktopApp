# Architecture Guide

Complete technical overview of GadAI's design, workflow, and codebase structure.

## System Overview

GadAI is a modular desktop chat application built with C++17/Qt6. The architecture follows a layered approach with clear separation of concerns.

### Design Principles
- **Modular:** Independent components with well-defined interfaces
- **Event-Driven:** Qt signals/slots for loose coupling
- **Extensible:** Plugin architecture for AI providers
- **Cross-Platform:** Single codebase for Windows, macOS, Linux

## Application Workflow

### 1. Startup Sequence
```
main.cpp → Application.init() → Services startup → UI initialization
```

1. **main.cpp** initializes Qt application
2. **Application** core starts all services
3. **LoginWindow** appears for authentication
4. **MainWindow** loads after successful login
5. **Event loop** begins processing user interactions

### 2. Authentication Flow
```
LoginWindow → AuthenticationService → Network/Local validation → Session creation
```

1. User enters credentials in **LoginWindow**
2. **AuthenticationService** validates against remote API or local demo
3. Success creates session token and user profile
4. **MainWindow** is displayed with authenticated context

### 3. Chat Message Flow
```
User input → MainWindow → Provider selection → AI processing → Response display
```

1. User types message in **MainWindow** input field
2. Message routed to selected **AI Provider** (EchoProvider for demo)
3. Provider processes request and generates response
4. Response displayed in conversation area

## Project Structure

```
GadAI/
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

## Core Components

### Application Core (`src/core/`)

**Application.h/.cpp**
- Main application coordinator and service container
- Initializes all services in correct order
- Manages application lifecycle and cleanup
- Provides service discovery and dependency injection

**main.cpp**
- Program entry point and Qt application setup
- Handles authentication flow and window management
- Event loop initialization and exception handling

### User Interface (`src/ui/`)

**LoginWindow.h/.cpp**
- Authentication interface with login/registration forms
- Handles username/password input and validation
- Connects to AuthenticationService for credential verification
- Supports both remote authentication and local demo mode

**MainWindow.h/.cpp**
- Primary chat interface with conversation management
- Left panel: conversation list and navigation
- Center area: message display and chat history
- Bottom section: message input and send controls
- Menu bar: settings, themes, and application controls

**Dialogs/** (various dialog classes)
- Settings configuration dialog
- About application information
- Error and confirmation dialogs

### Services Layer (`src/services/`)

**AuthenticationService.h/.cpp**
- User authentication and session management
- Remote API communication for login/registration
- Local demo user validation for development
- Session token management and automatic refresh
- Password hashing and security utilities

**SettingsStore.h/.cpp**
- Application configuration persistence
- User preferences (theme, window size, etc.)
- Cross-platform settings storage using Qt
- Settings validation and migration

**AudioRecording.h/.cpp** (planned)
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

**Storage.h/.cpp** (planned)
- Local database for conversation history
- Message persistence and retrieval
- Data encryption for sensitive information

## Key Architectural Patterns

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
- **Targets:** Separate library (GadAILib) and executable (GadAI)
- **Dependencies:** Automatic Qt component detection

### Library Structure
- **GadAILib:** Static library containing all core functionality
- **GadAI:** Minimal executable that links to library
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
