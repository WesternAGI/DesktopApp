#!/bin/bash
# DesktopApp Development Setup Script
# Supports Windows (via Git Bash/WSL), macOS, and Linux

set -e

echo "🚀 DesktopApp Development Setup"
echo "=============================="

# Detect operating system
OS="unknown"
case "$(uname -s)" in
    Linux*)     OS="linux";;
    Darwin*)    OS="macos";;
    CYGWIN*|MINGW*|MSYS*) OS="windows";;
esac

echo "Detected OS: $OS"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install Qt6 on different platforms
install_qt6() {
    echo "📦 Installing Qt6..."
    
    case $OS in
        "linux")
            if command_exists apt; then
                # Ubuntu/Debian
                sudo apt update
                sudo apt install -y qt6-base-dev qt6-multimedia-dev qt6-svg-dev \
                                    qt6-tools-dev qt6-tools-dev-tools \
                                    libqt6sql6-sqlite libqt6texttospeech6-dev
            elif command_exists dnf; then
                # Fedora
                sudo dnf install -y qt6-qtbase-devel qt6-qtmultimedia-devel \
                                    qt6-qtsvg-devel qt6-qttools-devel \
                                    qt6-qtsql qt6-qttexttospeech-devel
            elif command_exists pacman; then
                # Arch Linux
                sudo pacman -S qt6-base qt6-multimedia qt6-svg qt6-tools \
                              qt6-texttospeech
            else
                echo "❌ Unsupported Linux distribution"
                echo "Please install Qt6 manually from qt.io"
                exit 1
            fi
            ;;
        "macos")
            if command_exists brew; then
                brew install qt@6
                echo "Add Qt6 to your PATH:"
                echo 'export PATH="/usr/local/opt/qt@6/bin:$PATH"'
                echo 'export Qt6_DIR="/usr/local/opt/qt@6/lib/cmake/Qt6"'
            else
                echo "❌ Homebrew not found"
                echo "Please install Homebrew from brew.sh first"
                exit 1
            fi
            ;;
        "windows")
            echo "Please install Qt6 from qt.io"
            echo "Recommended: Qt 6.5+ with MinGW or MSVC compiler"
            echo "Add Qt6 to your PATH and set Qt6_DIR environment variable"
            ;;
    esac
}

# Function to install CMake
install_cmake() {
    echo "🔧 Installing CMake..."
    
    case $OS in
        "linux")
            if command_exists apt; then
                sudo apt install -y cmake
            elif command_exists dnf; then
                sudo dnf install -y cmake
            elif command_exists pacman; then
                sudo pacman -S cmake
            fi
            ;;
        "macos")
            if command_exists brew; then
                brew install cmake
            fi
            ;;
        "windows")
            echo "Please install CMake from cmake.org"
            ;;
    esac
}

# Function to install development tools
install_dev_tools() {
    echo "🛠️ Installing development tools..."
    
    case $OS in
        "linux")
            if command_exists apt; then
                sudo apt install -y build-essential git pkg-config \
                                    libssl-dev libsecret-1-dev
            elif command_exists dnf; then
                sudo dnf install -y gcc-c++ git pkgconfig \
                                    openssl-devel libsecret-devel
            elif command_exists pacman; then
                sudo pacman -S base-devel git pkgconf openssl libsecret
            fi
            ;;
        "macos")
            # Xcode command line tools
            xcode-select --install 2>/dev/null || true
            if command_exists brew; then
                brew install git pkg-config openssl
            fi
            ;;
        "windows")
            echo "Please install:"
            echo "- Git for Windows"
            echo "- Visual Studio 2019+ or MinGW-w64"
            echo "- OpenSSL (can be included with Qt)"
            ;;
    esac
}

# Check prerequisites
echo "🔍 Checking prerequisites..."

# Check Git
if ! command_exists git; then
    echo "❌ Git not found"
    install_dev_tools
fi

# Check CMake
if ! command_exists cmake; then
    echo "❌ CMake not found"
    install_cmake
fi

# Check for CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+')
CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d. -f1)
CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d. -f2)

if [ "$CMAKE_MAJOR" -lt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -lt 21 ]); then
    echo "❌ CMake 3.21+ required, found $CMAKE_VERSION"
    echo "Please update CMake"
    exit 1
fi

echo "✅ CMake $CMAKE_VERSION found"

# Check Qt6
if ! command_exists qmake6 && ! command_exists qmake && [ -z "$Qt6_DIR" ]; then
    echo "❌ Qt6 not found"
    read -p "Install Qt6? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_qt6
    else
        echo "Please install Qt6 manually and set Qt6_DIR environment variable"
        exit 1
    fi
fi

# Find Qt6 installation
echo "🔍 Looking for Qt6..."

QT6_PATHS=(
    "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
    "/usr/lib64/cmake/Qt6"
    "/usr/local/opt/qt@6/lib/cmake/Qt6"
    "/opt/Qt/6.*/gcc_64/lib/cmake/Qt6"
    "/opt/Qt/6.*/macos/lib/cmake/Qt6"
    "C:/Qt/6.*/msvc*/lib/cmake/Qt6"
    "C:/Qt/6.*/mingw*/lib/cmake/Qt6"
)

if [ -z "$Qt6_DIR" ]; then
    for path in "${QT6_PATHS[@]}"; do
        if [ -d "$path" ]; then
            export Qt6_DIR="$path"
            break
        fi
    done
fi

if [ -n "$Qt6_DIR" ]; then
    echo "✅ Qt6 found at: $Qt6_DIR"
else
    echo "❌ Qt6 not found automatically"
    echo "Please set Qt6_DIR environment variable to Qt6 cmake directory"
    echo "Example: export Qt6_DIR=/opt/Qt/6.5.0/gcc_64/lib/cmake/Qt6"
    exit 1
fi

# Create build directory
echo "📁 Setting up build directory..."
mkdir -p build
cd build

# Configure project
echo "⚙️ Configuring project..."
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_PREFIX_PATH="$Qt6_DIR" \
         -DENABLE_TESTS=ON

if [ $? -eq 0 ]; then
    echo "✅ Configuration successful"
else
    echo "❌ Configuration failed"
    echo "Check the error messages above"
    exit 1
fi

# Build project
echo "🔨 Building project..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo "✅ Build successful"
else
    echo "❌ Build failed"
    echo "Check the error messages above"
    exit 1
fi

# Run tests if available
if [ -f "./tests/DesktopApp_Tests" ] || [ -f "./tests/Debug/DesktopApp_Tests.exe" ]; then
    echo "🧪 Running tests..."
    ctest --output-on-failure
fi

echo ""
echo "🎉 Setup complete!"
echo "==================="
echo ""
echo "To run DesktopApp:"
if [ -d "Debug" ]; then
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        echo "  ./Debug/DesktopApp.exe"
    else
        echo "  ./Debug/DesktopApp"
    fi
else
        echo "  ./DesktopApp"
echo ""
echo "For development:"
echo "  - Use Qt Creator or VS Code with C++ extensions"
echo "  - Enable debug mode for detailed logging"
echo "  - Check docs/ for architecture and API documentation"
echo ""
echo "Next steps:"
echo "  1. Configure AI provider in Settings → AI Providers"
echo "  2. Try the Echo provider for offline testing"
echo "  3. Create your first conversation and start chatting!"
echo ""
echo "Need help? Check docs/USER_GUIDE.md or visit the repository"
