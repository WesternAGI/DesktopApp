# DesktopApp DLL Deployment Script
# This script uses windeployqt6 to automatically deploy all required Qt DLLs and plugins

param(
    [string]$BuildDir = "build",
    [string]$BuildType = "Debug",
    [switch]$NoTranslations
)

# Add MSYS2 UCRT64 to PATH
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

$targetDir = Join-Path $PSScriptRoot $BuildDir
$exePath = Join-Path $targetDir "DesktopApp.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found: $exePath" -ForegroundColor Red
    Write-Host "Please build the project first." -ForegroundColor Yellow
    exit 1
}

Write-Host "Deploying Qt dependencies to: $targetDir" -ForegroundColor Green
Write-Host "Executable: $exePath" -ForegroundColor Cyan
Write-Host ""

# Build windeployqt6 arguments
$deployArgs = @()
if ($NoTranslations) {
    $deployArgs += "--no-translations"
}

# Run windeployqt6
Write-Host "Running windeployqt6..." -ForegroundColor Cyan
$deployCmd = "windeployqt6 $($deployArgs -join ' ') `"$exePath`""
Invoke-Expression $deployCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "✓ Deployment complete!" -ForegroundColor Green
    Write-Host "You can now run DesktopApp.exe directly from the build directory." -ForegroundColor White
    Write-Host ""
    Write-Host "Test the deployment:" -ForegroundColor Yellow
    Write-Host "  cd $targetDir" -ForegroundColor Gray
    Write-Host "  .\DesktopApp.exe" -ForegroundColor Gray
} else {
    Write-Host ""
    Write-Host "✗ Deployment failed!" -ForegroundColor Red
    exit 1
}
