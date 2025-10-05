# DesktopApp Launcher Script
# This script ensures all required DLLs are accessible before running the application

param(
    [string]$BuildType = "Debug",
    [switch]$SkipAuth
)

# Add MSYS2 UCRT64 bin to PATH so DLLs can be found
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

# Determine executable path
$exePath = if ($BuildType -eq "Release") {
    ".\build-release\DesktopApp.exe"
} else {
    ".\build\DesktopApp.exe"
}

# Check if executable exists
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found at $exePath" -ForegroundColor Red
    Write-Host "Please build the project first." -ForegroundColor Yellow
    exit 1
}

Write-Host "Launching DesktopApp..." -ForegroundColor Green
Write-Host "Executable: $exePath" -ForegroundColor Cyan
Write-Host "DLL Path: C:\msys64\ucrt64\bin" -ForegroundColor Cyan

# Build arguments
$args = @()
if ($SkipAuth) {
    $args += "--skip-auth"
}

# Run the application
& $exePath $args
