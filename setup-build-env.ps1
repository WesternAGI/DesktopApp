# DesktopApp Build Environment Setup Script
# This script sets up the MSYS2 UCRT64 environment for building DesktopApp

Write-Host "Setting up DesktopApp build environment..." -ForegroundColor Green

# Add MSYS2 UCRT64 to PATH
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

# Verify tools are available
Write-Host "`nVerifying build tools:" -ForegroundColor Cyan
Write-Host "  CMake version: " -NoNewline
cmake --version | Select-Object -First 1
Write-Host "  Qt6 version: " -NoNewline
qmake6 --version | Select-Object -First 2 | Select-Object -Last 1
Write-Host "  GCC version: " -NoNewline
g++ --version | Select-Object -First 1

Write-Host "`nBuild environment ready!" -ForegroundColor Green
Write-Host "You can now run:" -ForegroundColor Yellow
Write-Host "  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug" -ForegroundColor White
Write-Host "  cmake --build build" -ForegroundColor White
Write-Host "  .\build\DesktopApp.exe" -ForegroundColor White
