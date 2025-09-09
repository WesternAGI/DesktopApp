# GadAI

Hello! Welcome to the GadAI project.

A simple desktop chat application built with C++17 and Qt6. This is a prototype with basic login functionality and a clean chat interface.

## What Works Right Now
- ✅ Online authentication
- ✅ Main chat window with conversation list
- ✅ Theme management (light/dark modes)
- ✅ Basic settings and preferences
- ✅ Audio recording capabilities
- ✅ Cross-platform support (Windows, macOS, Linux)

## What's Not Ready Yet
- ❌ Real AI chat providers (only demo/echo responses)
- ❌ File attachments and sharing
- ❌ Search functionality
- ❌ Data encryption
- ❌ Export/import features

## Prerequisites

Before you start, you need these installed on your computer:

### For All Platforms
- **Qt6** (version 6.2 or newer)
- **CMake** (version 3.21 or newer)
- **C++17 compatible compiler**

### Platform-Specific Requirements

**Windows:**
- Visual Studio 2019+ OR MinGW-w64
- Qt6 for Windows

**macOS:**
- Xcode Command Line Tools
- Qt6 for macOS

**Linux (Ubuntu/Debian):**
- GCC 9+ or Clang 10+
- Qt6 development packages

## Installation

### Step 1: Install Qt6

**Windows (using winget):**
```cmd
winget install Qt.Qt
```

**macOS (using Homebrew):**
```bash
brew install qt
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-svg-dev cmake build-essential
```

**Linux (Fedora):**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qtsvg-devel cmake gcc-c++
```

### Step 2: Download GadAI

**Option A: Download ZIP**
1. Click the green "Code" button on GitHub
2. Select "Download ZIP"
3. Extract to a folder like `C:\GadAI` or `~/GadAI`

**Option B: Use Git**
```bash
git clone https://github.com/WesternAGI/GadAI.git
cd GadAI
```

### Step 3: Build the Application

**Windows (PowerShell):**
```powershell
# Open PowerShell in the GadAI folder
# Replace C:\Qt\6.5.0\msvc2019_64 with your Qt installation path
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
cmake --build build --config Release
```

**Windows (with MinGW):**
```cmd
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"
cmake --build build
```

**macOS:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
```

**Linux:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Step 4: Run the Application

**Windows:**
```powershell
.\build\GadAI.exe
```

**macOS/Linux:**
```bash
./build/GadAI
```

## Using GadAI

### First Time Login
When you start the app, use one of these demo accounts:
- **Phone:** `+15550000001` **Password:** `demo123`
- **Phone:** `+15550000002` **Password:** `admin123`

### Skip Login (For Development)
If you want to skip the login screen:
```bash
./build/GadAI --skip-auth
```

### Basic Usage
1. **Login** with a demo account
2. **Start chatting** in the main window
3. **Change themes** using the theme toggle button
4. **Access settings** from the menu bar

## Troubleshooting

### Common Problems and Solutions

**Problem: "Qt not found" error**
- **Solution:** Make sure you set the correct path to Qt in the cmake command
- **Windows:** Use the full path like `C:\Qt\6.5.0\msvc2019_64`
- **macOS:** Try `$(brew --prefix qt)` or `/usr/local/opt/qt`
- **Linux:** Usually `/usr/lib/qt6` or `/usr/lib/x86_64-linux-gnu/qt6`

**Problem: Missing DLL files on Windows**
- **Solution:** Copy Qt DLL files to the build folder, or add Qt bin folder to your PATH
- **Location:** Usually in `C:\Qt\6.5.0\msvc2019_64\bin`

**Problem: App crashes after login**
- **Solution:** Try running with `--skip-auth` to bypass login
- **Alternative:** Delete settings files in `%APPDATA%\GadAI Project\GadAI`

**Problem: Build fails**
- **Solution:** Delete the `build` folder and try again
- **Alternative:** Make sure you have all required dependencies installed

### Getting Help
- Check that all prerequisites are installed correctly
- Make sure you're using the exact Qt path from your installation
- Try building a simple Qt example first to verify your setup
- Delete the `build` folder and start fresh if you get weird errors

## Project Structure

```
GadAI/
├── src/
│   ├── core/           # Application startup and core services
│   ├── ui/             # User interface (login, main window, dialogs)
│   ├── services/       # Authentication, settings, audio recording
│   ├── providers/      # AI provider interfaces (demo only)
│   ├── theme/          # Theme and icon management
│   └── data/           # Data models and storage
├── docs/               # Additional documentation
├── icons/              # Application icons
├── scripts/            # Build and setup scripts
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

## Development

### Building for Development
```bash
# Debug build
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/path/to/qt6"
cmake --build build-debug

# Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="/path/to/qt6"
cmake --build build
```

### Adding Features
1. Core services go in `src/services/`
2. UI components go in `src/ui/`
3. Update `CMakeLists.txt` to include new source files
4. Follow Qt's object system patterns

## License
MIT License - feel free to use and modify

## Contributing
This is a prototype project. Feel free to:
- Report bugs and issues
- Suggest improvements
- Submit pull requests
- Fork for your own projects

---
