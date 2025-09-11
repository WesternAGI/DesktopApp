# DesktopApp

A desktop chat application built with C++17 and Qt6.

## Features
- ✅ Login system with online authentication
- ✅ Chat interface with conversation list
- ✅ Light/dark theme support
- ✅ Cross-platform (Windows, macOS, Linux)
- ❌ AI providers (demo only)
- ❌ File attachments
- ❌ Message search

## Quick Start

### Requirements
- Qt6 (6.2+)
- CMake (3.21+)
- C++17 compiler
- Internet connection for authentication

### Build & Run
```bash
git clone https://github.com/WesternAGI/DesktopApp.git
cd DesktopApp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/DesktopApp  # or .\build\DesktopApp.exe on Windows
```

## Documentation

- **[Installation Guide](docs/INSTALLATION.md)** - Detailed setup instructions for all platforms
- **[Architecture Guide](docs/ARCHITECTURE.md)** - Technical overview and project structure
- **[User Guide](docs/USER_GUIDE.md)** - How to use the application
- **[Security Guide](docs/SECURITY.md)** - Security implementation details
- **[Project Status](docs/PROJECT_STATUS.md)** - Current development status

## License
MIT License
