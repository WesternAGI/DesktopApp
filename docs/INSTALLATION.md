# Installation Guide

Step-by-step instructions to build and run DesktopApp on Windows, macOS, and Linux.

## Prerequisites

### Required Software
- **Qt6** (version 6.2 or newer)
- **CMake** (version 3.21 or newer)  
- **C++17 compatible compiler**
- **Git** (to clone the repository)

## Step 1: Install Dependencies

### Windows

**Option A: Using winget (Recommended)**
```cmd
winget install Qt.Qt
winget install Kitware.CMake
winget install Git.Git
```

**Option B: Manual Installation**
1. Download Qt6 from [qt.io](https://www.qt.io/download)
2. Download CMake from [cmake.org](https://cmake.org/download/)
3. Install Visual Studio 2019+ or MinGW-w64

### macOS

**Using Homebrew (Recommended)**
```bash
brew install qt cmake git
```

**Manual Installation**
1. Install Xcode Command Line Tools: `xcode-select --install`
2. Download Qt6 from [qt.io](https://www.qt.io/download)
3. Download CMake from [cmake.org](https://cmake.org/download/)

### Linux

**Ubuntu/Debian**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-svg-dev cmake build-essential git
```

**Fedora**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qtsvg-devel cmake gcc-c++ git
```

**Arch Linux**
```bash
sudo pacman -S qt6-base qt6-multimedia qt6-svg cmake gcc git
```

## Step 2: Clone Repository

```bash
git clone https://github.com/WesternAGI/DesktopApp.git
cd DesktopApp
```

## Step 3: Build Application

### Windows (PowerShell)

**With Visual Studio**
```powershell
cmake -S . -B build -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

**With MinGW**
```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### macOS
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
```

### Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Step 4: Run Application

### Windows
```cmd
.\build\Release\DesktopApp.exe
# or for MinGW build:
.\build\DesktopApp.exe
```

### macOS/Linux
```bash
./build/DesktopApp
```

### Development Mode
To skip authentication during development:
```bash
./build/DesktopApp --skip-auth
```

## Troubleshooting

### Common Build Issues

**Qt not found error**
- **Windows**: Set Qt path: `-DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"`
- **macOS**: Ensure Homebrew Qt is in PATH: `export PATH=$(brew --prefix qt)/bin:$PATH`
- **Linux**: Install development packages: `sudo apt install qt6-base-dev`

**Missing DLL files (Windows)**
- Add Qt bin directory to system PATH: `C:\Qt\6.5.0\msvc2019_64\bin`
- Or copy required DLLs to the build directory

**Build fails with compiler errors**
- Delete build directory and retry: `rm -rf build` (Linux/macOS) or `rmdir /s build` (Windows)
- Verify all dependencies are installed correctly
- Check that CMake and compiler versions meet requirements

### Runtime Issues

**Application crashes on startup**
- Try development mode: `./build/DesktopApp --skip-auth`
- Check Qt installation and runtime libraries
- Verify all required DLLs are available (Windows)

**Login fails**
- Use demo credentials: username `demo`, password `demo123`
- Try skipping authentication with `--skip-auth` flag
- Check internet connection if using online authentication

**Settings not saved**
- Ensure application has write permissions to settings directory
- Delete settings to reset: 
  - Windows: `%APPDATA%\DesktopApp Project\`
  - macOS/Linux: `~/.config/DesktopApp Project/`

## Development Build

For development with debug symbols and verbose output:
```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

## Platform-Specific Notes

### Windows
- Requires Visual Studio 2019+ or MinGW-w64
- Qt DLLs must be in PATH or copied to build directory
- May need to run as administrator for first installation

### macOS
- Requires Xcode Command Line Tools
- Homebrew is recommended for dependency management
- Application bundle will be created in the build directory

### Linux
- Package names vary between distributions
- Some distributions may require additional Qt modules
- Desktop integration works with most modern desktop environments

---

For additional help, see the [User Guide](USER_GUIDE.md) and [Project Status](PROJECT_STATUS.md).
