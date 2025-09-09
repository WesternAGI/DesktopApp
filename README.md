# GadAI

A desktop chat application built with C++17 and Qt6.

## Features
- ✅ Login system with authentication
- ✅ Chat interface with conversation list
- ✅ Light/dark theme support
- ✅ Cross-platform (Windows, macOS, Linux)
- ❌ AI providers (demo only)
- ❌ File attachments
- ❌ Message search

## Requirements
- Qt6 (6.2+)
- CMake (3.21+)
- C++17 compiler

## Quick Start

### Install Qt6
**Windows:** `winget install Qt.Qt`
**macOS:** `brew install qt`
**Linux:** `sudo apt install qt6-base-dev qt6-multimedia-dev cmake`

### Build & Run
```bash
git clone https://github.com/WesternAGI/GadAI.git
cd GadAI
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/GadAI  # or .\build\GadAI.exe on Windows
```

### Demo Login
- Username: `demo` Password: `demo123`

## Project Structure
```
src/
├── core/           # Application startup
├── ui/             # Login and main windows
├── services/       # Authentication, settings
├── providers/      # AI interfaces (demo)
└── theme/          # Themes and icons
```

## License
MIT License
