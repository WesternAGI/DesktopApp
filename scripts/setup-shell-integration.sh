#!/bin/bash
# DesktopApp Shell Integration Setup Script
# This script enables shell integration for better command detection on Unix-like systems

set -e

echo "🔧 Setting up Shell Integration for DesktopApp"
echo "==============================================="

# Detect shell
CURRENT_SHELL=$(basename "$SHELL")
echo "Detected shell: $CURRENT_SHELL"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Configure Git for better shell integration
if command_exists git; then
    echo "✅ Git found - configuring Git integration"
    
    # Configure Git for better terminal integration
    git config --global core.autocrlf input
    git config --global core.editor "code --wait" 2>/dev/null || git config --global core.editor "nano"
    
    # Enable Git credential helper if available
    if command_exists git-credential-manager-core; then
        git config --global credential.helper manager-core
    elif command_exists git-credential-manager; then
        git config --global credential.helper manager
    fi
    
    echo "✅ Git configured for shell integration"
else
    echo "⚠️  Git not found - please install git for better shell features"
fi

# Configure VS Code if available
if command_exists code; then
    echo "✅ VS Code found - configuring shell integration"
    
    # Install recommended extensions
    echo "📦 Installing recommended VS Code extensions..."
    code --install-extension ms-vscode.cpptools 2>/dev/null || echo "⚠️  Could not install C++ extension"
    code --install-extension ms-vscode.cmake-tools 2>/dev/null || echo "⚠️  Could not install CMake Tools"
    code --install-extension twxs.cmake 2>/dev/null || echo "⚠️  Could not install CMake extension"
    
    echo "✅ VS Code extensions installed"
else
    echo "⚠️  VS Code not found - install for better development experience"
fi

# Setup shell-specific configurations
setup_bash() {
    echo "📝 Configuring Bash integration..."
    
    # Create or append to .bashrc
    BASHRC="$HOME/.bashrc"
    if [ ! -f "$BASHRC" ]; then
        touch "$BASHRC"
    fi
    
    # Add DesktopApp aliases and functions
    cat >> "$BASHRC" << 'EOF'

# DesktopApp Development Aliases
alias build='cmake --build build'
alias clean='rm -rf build && mkdir build'
alias run='./build/DesktopApp'
alias gst='git status'
alias gco='git checkout'
alias gcm='git commit -m'

# DesktopApp Development Functions
dev-build() {
    local config=${1:-Debug}
    echo "🔨 Building DesktopApp ($config)..."
    
    if [ "$1" = "clean" ]; then
        rm -rf build
    fi
    
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE="$config"
    make -j$(nproc)
    cd ..
}

dev-run() {
    local config=${1:-Debug}
    if [ -f "build/DesktopApp" ]; then
        echo "🚀 Running DesktopApp..."
        ./build/DesktopApp "$@"
    else
        echo "❌ Executable not found. Build the project first."
        echo "Run: dev-build"
    fi
}

dev-help() {
    echo "DesktopApp Development Commands:"
    echo "  dev-build [Debug|Release|clean] - Build the project"
    echo "  dev-run [args]                  - Run the application"
    echo "  build                           - Quick build"
    echo "  clean                           - Clean build artifacts"
    echo "  run                             - Quick run"
    echo "  gst, gco, gcm                   - Git shortcuts"
}

EOF
    
    echo "✅ Bash configuration added to $BASHRC"
}

setup_zsh() {
    echo "📝 Configuring Zsh integration..."
    
    # Create or append to .zshrc
    ZSHRC="$HOME/.zshrc"
    if [ ! -f "$ZSHRC" ]; then
        touch "$ZSHRC"
    fi
    
    # Add DesktopApp aliases and functions (similar to bash)
    cat >> "$ZSHRC" << 'EOF'

# DesktopApp Development Environment
autoload -U compinit && compinit

# Aliases
alias build='cmake --build build'
alias clean='rm -rf build && mkdir build'
alias run='./build/DesktopApp'
alias gst='git status'
alias gco='git checkout'
alias gcm='git commit -m'

# Functions
dev-build() {
    local config=${1:-Debug}
    echo "🔨 Building DesktopApp ($config)..."
    
    if [[ "$1" == "clean" ]]; then
        rm -rf build
    fi
    
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE="$config"
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cd ..
}

dev-run() {
    if [[ -f "build/DesktopApp" ]]; then
        echo "🚀 Running DesktopApp..."
        ./build/DesktopApp "$@"
    else
        echo "❌ Executable not found. Build the project first."
        echo "Run: dev-build"
    fi
}

dev-help() {
    echo "DesktopApp Development Commands:"
    echo "  dev-build [Debug|Release|clean] - Build the project"
    echo "  dev-run [args]                  - Run the application"
    echo "  build, clean, run               - Quick commands"
    echo "  gst, gco, gcm                   - Git shortcuts"
}

# Auto-completion
_dev_build_completion() {
    _arguments '1:config:(Debug Release clean)'
}
compdef _dev_build_completion dev-build

EOF
    
    echo "✅ Zsh configuration added to $ZSHRC"
}

# Configure shell-specific features
case "$CURRENT_SHELL" in
    bash)
        setup_bash
        ;;
    zsh)
        setup_zsh
        ;;
    *)
        echo "⚠️  Shell $CURRENT_SHELL not specifically supported, but basic features should work"
        ;;
esac

# Setup environment variables
echo "📁 Setting up environment variables..."

# Try to find Qt6
QT6_DIRS=(
    "/usr/lib/qt6"
    "/usr/local/lib/qt6"
    "/opt/qt6"
    "/opt/Qt/6.5.0/gcc_64"
    "/opt/Qt/6.6.0/gcc_64"
    "/opt/Qt/6.7.0/gcc_64"
    "$(brew --prefix qt6 2>/dev/null || echo)"
)

QT6_FOUND=0
for qt_dir in "${QT6_DIRS[@]}"; do
    if [ -d "$qt_dir" ] && [ -f "$qt_dir/lib/cmake/Qt6/Qt6Config.cmake" ]; then
        echo "✅ Found Qt6 at $qt_dir"
        export Qt6_DIR="$qt_dir/lib/cmake/Qt6"
        echo "export Qt6_DIR=\"$qt_dir/lib/cmake/Qt6\"" >> "$HOME/.bashrc" 2>/dev/null || true
        echo "export Qt6_DIR=\"$qt_dir/lib/cmake/Qt6\"" >> "$HOME/.zshrc" 2>/dev/null || true
        QT6_FOUND=1
        break
    fi
done

if [ $QT6_FOUND -eq 0 ]; then
    echo "⚠️  Qt6 not found in standard locations"
    echo "Please set Qt6_DIR environment variable manually"
fi

# Check CMake
if command_exists cmake; then
    echo "✅ CMake found"
    # Set preferred generator
    export CMAKE_GENERATOR="Ninja"
    echo "export CMAKE_GENERATOR=\"Ninja\"" >> "$HOME/.bashrc" 2>/dev/null || true
    echo "export CMAKE_GENERATOR=\"Ninja\"" >> "$HOME/.zshrc" 2>/dev/null || true
else
    echo "⚠️  CMake not found - please install CMake 3.21+"
fi

# Create project-specific shell integration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Create a project-specific activation script
cat > "$PROJECT_DIR/.vscode/shell_integration.sh" << 'EOF'
#!/bin/bash
# DesktopApp Shell Integration
# Source this file to enable project-specific shell features

# Check if we're in the DesktopApp project directory
if [ -f "CMakeLists.txt" ] && grep -q "DesktopApp" CMakeLists.txt 2>/dev/null; then
    echo "🎯 DesktopApp development environment loaded"
    echo "Type 'dev-help' for available commands"
    
    # Add Qt6 to PATH if available
    if [ -n "$Qt6_DIR" ]; then
        QT_BIN_DIR="$(dirname "$(dirname "$Qt6_DIR")")/bin"
        if [ -d "$QT_BIN_DIR" ] && [[ ":$PATH:" != *":$QT_BIN_DIR:"* ]]; then
            export PATH="$QT_BIN_DIR:$PATH"
            echo "✅ Added Qt6 to PATH: $QT_BIN_DIR"
        fi
    fi
fi
EOF

chmod +x "$PROJECT_DIR/.vscode/shell_integration.sh"

echo ""
echo "🎉 Shell Integration Setup Complete!"
echo "===================================="
echo ""
echo "Features enabled:"
echo "  ✅ Enhanced command detection in terminals"
echo "  ✅ Shell integration with custom commands"
echo "  ✅ Git integration and configuration"
echo "  ✅ VS Code integration (if available)"
echo "  ✅ Project-specific environment variables"
echo "  ✅ CMake and build tool integration"
echo ""
echo "Next steps:"
echo "  1. Restart your terminal for changes to take effect"
echo "  2. Use 'dev-help' for development commands"
echo "  3. Try building with: dev-build"
echo "  4. Run the app with: dev-run"
echo ""
echo "For immediate activation in current session:"
echo "  source ~/.${CURRENT_SHELL}rc"
echo "  source .vscode/shell_integration.sh"
echo ""