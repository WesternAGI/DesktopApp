# Complete DLL Deployment Script
# This script copies ALL dependencies needed for Qt6 applications from MSYS2

param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Continue"
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

$targetDir = Join-Path $PSScriptRoot $BuildDir
$exePath = Join-Path $targetDir "DesktopApp.exe"
$msys64Bin = "C:\msys64\ucrt64\bin"

if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found: $exePath" -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Complete DLL Deployment for DesktopApp" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Target: $targetDir" -ForegroundColor Green
Write-Host "Source: $msys64Bin" -ForegroundColor Green
Write-Host ""

# Step 1: Run windeployqt6
Write-Host "[1/3] Running windeployqt6..." -ForegroundColor Cyan
$deployCmd = "windeployqt6 --force --no-translations `"$exePath`""
Invoke-Expression $deployCmd | Out-Null

# Step 2: Copy ALL potential Qt dependencies
Write-Host ""
Write-Host "[2/3] Copying all Qt dependencies..." -ForegroundColor Cyan

$allDependencies = @(
    # MinGW Runtime (Critical)
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    
    # Qt Core Dependencies
    "libdouble-conversion.dll",
    "libicudt77.dll",
    "libicuin77.dll",
    "libicuuc77.dll",
    "libpcre2-16-0.dll",
    "libzstd.dll",
    "libb2-1.dll",
    
    # Qt GUI Dependencies
    "libfreetype-6.dll",
    "libharfbuzz-0.dll",
    "libpng16-16.dll",
    "libjpeg-8.dll",
    "libmd4c.dll",
    "libgraphite2.dll",
    
    # GLib and dependencies
    "libglib-2.0-0.dll",
    "libintl-8.dll",
    "libiconv-2.dll",
    "libpcre2-8-0.dll",
    
    # Font rendering
    "libbz2-1.dll",
    "libbrotlidec.dll",
    "libbrotlicommon.dll",
    
    # Compression
    "zlib1.dll",
    
    # SSL/TLS (for Network)
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll",
    
    # DBus (for Qt GUI on Windows)
    "libdbus-1-3.dll",
    
    # Additional libraries that Qt may need
    "libsqlite3-0.dll",
    "libexpat-1.dll",
    "libffi-8.dll",
    "libpixman-1-0.dll",
    "libcairo-2.dll",
    "libfontconfig-1.dll",
    "libxml2-2.dll",
    "liblzma-5.dll",
    
    # JPEG XL support
    "libjxl.dll",
    "libjxl_threads.dll",
    "libhwy.dll",
    
    # WebP support
    "libwebp-7.dll",
    "libwebpdemux-2.dll",
    "libwebpmux-3.dll",
    "libsharpyuv-0.dll",
    
    # TIFF support
    "libtiff-6.dll",
    "libLerc.dll",
    "libdeflate.dll",
    "libjbig-0.dll",
    
    # Additional image formats
    "libgif-7.dll",
    "libturbojpeg-0.dll",
    "libopenjp2-7.dll",
    
    # Multimedia dependencies
    "libspeex-1.dll",
    "libogg-0.dll",
    "libvorbis-0.dll",
    "libvorbisenc-2.dll",
    "libopus-0.dll",
    "libFLAC-12.dll",
    
    # Text rendering
    "libpango-1.0-0.dll",
    "libpangocairo-1.0-0.dll",
    "libpangoft2-1.0-0.dll",
    "libpangowin32-1.0-0.dll",
    
    # GObject system
    "libgobject-2.0-0.dll",
    "libgmodule-2.0-0.dll",
    "libgio-2.0-0.dll"
)

$copiedCount = 0
$skippedCount = 0

foreach ($dll in $allDependencies) {
    $sourcePath = Join-Path $msys64Bin $dll
    $destPath = Join-Path $targetDir $dll
    
    if (Test-Path $sourcePath) {
        if (-not (Test-Path $destPath)) {
            Copy-Item $sourcePath $destPath -Force
            Write-Host "  ✓ $dll" -ForegroundColor Green
            $copiedCount++
        } else {
            $skippedCount++
        }
    }
}

# Step 3: Summary
Write-Host ""
Write-Host "[3/3] Deployment Summary" -ForegroundColor Cyan
Write-Host "  New DLLs copied: $copiedCount" -ForegroundColor Green
Write-Host "  Already present: $skippedCount" -ForegroundColor Gray

$totalDlls = (Get-ChildItem "$targetDir\*.dll").Count
Write-Host "  Total DLLs in build: $totalDlls" -ForegroundColor White

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ Deployment Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Test the application:" -ForegroundColor Yellow
Write-Host "  cd $targetDir" -ForegroundColor Gray
Write-Host "  .\DesktopApp.exe" -ForegroundColor Gray
Write-Host ""
