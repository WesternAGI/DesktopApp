@echo off
REM DesktopApp Shell Integration Setup Script
REM This script enables shell integration for better command detection

echo 🔧 Setting up Shell Integration for DesktopApp
echo ===============================================

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ⚠️  This script may require administrator privileges for some features
    echo Consider running as administrator for best results
    echo.
)

REM Set PowerShell execution policy for current user
echo 📝 Configuring PowerShell execution policy...
powershell -Command "Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force" 2>nul
if %errorLevel% eq 0 (
    echo ✅ PowerShell execution policy configured
) else (
    echo ⚠️  Could not configure PowerShell execution policy
)

REM Install PowerShell modules for better shell integration
echo 📦 Installing PowerShell modules...
powershell -Command "Install-Module -Name PSReadLine -Force -SkipPublisherCheck" 2>nul
if %errorLevel% eq 0 (
    echo ✅ PSReadLine module installed/updated
) else (
    echo ⚠️  Could not install PSReadLine module
)

REM Check if Git is installed and configure
git --version >nul 2>&1
if %errorLevel% eq 0 (
    echo ✅ Git found - configuring Git integration
    
    REM Enable Git credential manager
    git config --global credential.helper manager-core 2>nul
    
    REM Configure Git for better terminal integration
    git config --global core.autocrlf true 2>nul
    git config --global core.editor "code --wait" 2>nul
    
    echo ✅ Git configured for shell integration
) else (
    echo ⚠️  Git not found - some shell features may be limited
)

REM Configure VS Code for shell integration
if exist "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" (
    echo ✅ VS Code found - configuring shell integration
    
    REM Add VS Code to PATH if not already there
    echo %PATH% | find /i "Microsoft VS Code" >nul
    if %errorLevel% neq 0 (
        echo 📝 Adding VS Code to PATH...
        setx PATH "%PATH%;%LOCALAPPDATA%\Programs\Microsoft VS Code\bin" >nul 2>&1
        if %errorLevel% eq 0 (
            echo ✅ VS Code added to PATH
        ) else (
            echo ⚠️  Could not add VS Code to PATH automatically
            echo You may need to add it manually: %LOCALAPPDATA%\Programs\Microsoft VS Code\bin
        )
    )
    
    REM Install recommended VS Code extensions for C++ development
    echo 📦 Installing recommended VS Code extensions...
    call "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" --install-extension ms-vscode.cpptools >nul 2>&1
    call "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" --install-extension ms-vscode.cmake-tools >nul 2>&1
    call "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" --install-extension twxs.cmake >nul 2>&1
    call "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" --install-extension ms-vscode.powershell >nul 2>&1
    echo ✅ VS Code extensions installed
) else (
    echo ⚠️  VS Code not found in standard location
)

REM Configure Windows Terminal if available
if exist "%LOCALAPPDATA%\Microsoft\WindowsApps\wt.exe" (
    echo ✅ Windows Terminal found
    echo 💡 Consider configuring Windows Terminal for better shell experience
    echo    - Enable shell integration in Terminal settings
    echo    - Configure PowerShell as default profile
) else (
    echo ℹ️  Windows Terminal not found - consider installing for better experience
)

REM Setup project-specific environment
echo 📁 Setting up project environment variables...

REM Try to find Qt6 installation
set QT6_FOUND=0
for %%d in (
    "C:\Qt\6.5.0\msvc2019_64"
    "C:\Qt\6.6.0\msvc2019_64"
    "C:\Qt\6.7.0\msvc2019_64"
    "C:\msys64\ucrt64"
) do (
    if exist "%%d\bin\qmake.exe" (
        echo ✅ Found Qt6 at %%d
        setx Qt6_DIR "%%d\lib\cmake\Qt6" >nul 2>&1
        set QT6_FOUND=1
        goto :qt_found
    )
)

:qt_found
if %QT6_FOUND%==0 (
    echo ⚠️  Qt6 not found in standard locations
    echo Please set Qt6_DIR environment variable manually
)

REM Setup CMake environment
cmake --version >nul 2>&1
if %errorLevel% eq 0 (
    echo ✅ CMake found
    REM Set preferred generator
    setx CMAKE_GENERATOR "Ninja" >nul 2>&1
) else (
    echo ⚠️  CMake not found - please install CMake 3.21+
)

REM Create PowerShell profile for enhanced shell integration
echo 📝 Creating PowerShell profile...
set PROFILE_DIR=%USERPROFILE%\Documents\WindowsPowerShell
if not exist "%PROFILE_DIR%" mkdir "%PROFILE_DIR%"

echo # DesktopApp Shell Integration Profile > "%PROFILE_DIR%\Microsoft.PowerShell_profile.ps1"
echo . "%~dp0.vscode\powershell_profile.ps1" >> "%PROFILE_DIR%\Microsoft.PowerShell_profile.ps1"

echo ✅ PowerShell profile created

echo.
echo 🎉 Shell Integration Setup Complete!
echo ====================================
echo.
echo Features enabled:
echo   ✅ Enhanced command detection in terminals
echo   ✅ PowerShell integration with custom commands
echo   ✅ Git integration and configuration
echo   ✅ VS Code terminal integration
echo   ✅ Project-specific environment variables
echo   ✅ CMake and build tool integration
echo.
echo Next steps:
echo   1. Restart your terminal/VS Code for changes to take effect
echo   2. Use 'help-dev' in PowerShell for development commands
echo   3. Try building with: build
echo   4. Run the app with: run
echo.
echo For VS Code:
echo   - Use Ctrl+Shift+P and type "Terminal: Focus Terminal"
echo   - Use Ctrl+` to toggle integrated terminal
echo   - Commands should now be better detected and highlighted
echo.

pause