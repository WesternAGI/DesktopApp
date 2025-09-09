@echo off
REM DesktopApp Development Setup Script for Windows
REM Requires PowerShell and administrator privileges for some operations

@echo off
REM DesktopApp Development Setup Script for Windows
setlocal

echo 🚀 DesktopApp Development Setup for Windows
echo ==========================================

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ⚠️ Some operations may require administrator privileges
    echo Consider running as administrator for best experience
)

REM Check for required tools
echo 🔍 Checking prerequisites...

REM Check Git
git --version >nul 2>&1
if %errorLevel% neq 0 (
    echo ❌ Git not found
    echo Please install Git for Windows from git-scm.com
    pause
    exit /b 1
)
echo ✅ Git found

REM Check CMake
cmake --version >nul 2>&1
if %errorLevel% neq 0 (
    echo ❌ CMake not found
    echo Installing CMake via winget...
    winget install Kitware.CMake
    if %errorLevel% neq 0 (
        echo ❌ Failed to install CMake
        echo Please install CMake manually from cmake.org
        pause
        exit /b 1
    )
)
echo ✅ CMake found

REM Check Qt6
qmake --version >nul 2>&1
if %errorLevel% neq 0 (
    echo ❌ Qt6 not found in PATH
    echo Checking common installation locations...
    
    set QT_FOUND=0
    for /d %%i in ("C:\Qt\6.*") do (
        if exist "%%i\msvc*\bin\qmake.exe" (
            set "Qt6_DIR=%%i\msvc*\lib\cmake\Qt6"
            set QT_FOUND=1
            goto :qt_found
        )
        if exist "%%i\mingw*\bin\qmake.exe" (
            set "Qt6_DIR=%%i\mingw*\lib\cmake\Qt6"
            set QT_FOUND=1
            goto :qt_found
        )
    )
    
    :qt_found
    if %QT_FOUND%==0 (
        echo ❌ Qt6 not found
        echo Please install Qt6 from qt.io
        echo Recommended: Qt 6.5+ with MSVC 2019 or MinGW compiler
        pause
        exit /b 1
    )
)
echo ✅ Qt6 found

REM Check Visual Studio or MinGW
cl >nul 2>&1
if %errorLevel% neq 0 (
    gcc --version >nul 2>&1
    if %errorLevel% neq 0 (
        echo ❌ No C++ compiler found
        echo Please install Visual Studio 2019+ or MinGW-w64
        pause
        exit /b 1
    ) else (
        echo ✅ MinGW compiler found
        set COMPILER=MinGW
    )
) else (
    echo ✅ MSVC compiler found
    set COMPILER=MSVC
)

REM Create build directory
echo 📁 Setting up build directory...
if not exist "build" mkdir build
cd build

REM Configure project
echo ⚙️ Configuring project...
if defined Qt6_DIR (
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="%Qt6_DIR%" -DENABLE_TESTS=ON
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON
)

if %errorLevel% neq 0 (
    echo ❌ Configuration failed
    echo Common issues:
    echo - Qt6 not found: Set Qt6_DIR environment variable
    echo - Missing dependencies: Install Visual Studio or MinGW
    echo - CMake version: Ensure CMake 3.21+ is installed
    pause
    exit /b 1
)
echo ✅ Configuration successful

REM Build project
echo 🔨 Building project...
cmake --build . --config Debug

if %errorLevel% neq 0 (
    echo ❌ Build failed
    echo Check the error messages above
    echo Common solutions:
    echo - Ensure Qt6 is properly installed
    echo - Check that all dependencies are available
    echo - Try cleaning the build directory and reconfiguring
    pause
    exit /b 1
)
echo ✅ Build successful

REM Run tests if available
if exist "tests\Debug\DesktopApp_Tests.exe" (
    echo 🧪 Running tests...
    ctest --output-on-failure -C Debug
)

echo.
echo 🎉 Setup complete!
echo ===================
echo.
echo To run DesktopApp:
echo   Debug\DesktopApp.exe
echo.
echo For development:
echo   - Use Visual Studio, Qt Creator, or VS Code with C++ extensions
echo   - Enable debug mode for detailed logging
echo   - Check docs\ for architecture and API documentation
echo.
echo Environment setup:
echo   - Add Qt6\bin to your PATH for easier development
echo   - Set Qt6_DIR to Qt6\lib\cmake\Qt6 if not auto-detected
echo.
echo Next steps:
echo   1. Configure AI provider in Settings → AI Providers
echo   2. Try the Echo provider for offline testing
echo   3. Create your first conversation and start chatting!
echo.
echo Need help? Check docs\USER_GUIDE.md or visit the repository

pause
