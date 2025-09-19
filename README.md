# DesktopApp

A modern desktop chat application built with C++17 and Qt6, featuring a clean ChatGPT-style interface with AI provider support.

## Features

- **User Authentication**: Secure login system with JWT token management
- **Provider System**: Support for multiple AI providers (Echo, Backend AI)
- **Modern UI**: Clean ChatGPT-inspired interface with enhanced styling
- **Message Management**: Robust conversation storage and retrieval
- **Theme Support**: Light/dark theme with consistent design tokens
- **Cross-platform**: Windows, macOS, Linux support
- **Real-time Communication**: Seamless messaging with AI providers
- **Secure Storage**: Encrypted credential storage

## Quick Start

### Requirements
- Qt6 (6.2 or newer)
- CMake (3.21 or newer)  
- C++17 compatible compiler
- Internet connection for Backend AI provider

### Build and Run
```bash
git clone https://github.com/WesternAGI/DesktopApp.git
cd DesktopApp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the application
./build/DesktopApp  # Linux/macOS
.\build\DesktopApp.exe  # Windows
```

### Provider Configuration
- **Echo Provider**: Built-in for testing, echoes your messages
- **Backend AI**: Connects to remote AI service with user authentication

## Documentation

- [Installation Guide](docs/INSTALLATION.md) - Detailed setup instructions for all platforms
- [Architecture Guide](docs/ARCHITECTURE.md) - Technical overview and project structure
- [User Guide](docs/USER_GUIDE.md) - How to use the application
- [Security Guide](docs/SECURITY.md) - Security implementation details
- [Project Status](docs/PROJECT_STATUS.md) - Current development status

## License

MIT License
