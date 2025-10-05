# Smart DLL Deployment - Copy Only Required Dependencies
# This script analyzes DLL dependencies and copies only what's needed

param(
    [string]$BuildDir = "build"
)

$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
$targetDir = Join-Path $PSScriptRoot $BuildDir
$exePath = Join-Path $targetDir "DesktopApp.exe"
$msys64Bin = "C:\msys64\ucrt64\bin"

if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found: $exePath" -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Smart DLL Deployment for DesktopApp" -ForegroundColor White
Write-Host "========================================`n" -ForegroundColor Cyan

# Step 1: Run windeployqt6
Write-Host "[Step 1/2] Running windeployqt6..." -ForegroundColor Cyan
windeployqt6 --force --no-translations "$exePath" 2>&1 | Out-Null
Write-Host "  ✓ Qt DLLs and plugins deployed`n" -ForegroundColor Green

# Step 2: Get list of missing DLLs by analyzing all DLLs in target directory
Write-Host "[Step 2/2] Analyzing and copying dependencies..." -ForegroundColor Cyan

# Get all DLLs currently in build directory
$existingDlls = Get-ChildItem "$targetDir\*.dll" -Recurse | Select-Object -ExpandProperty Name | Sort-Object -Unique

# Common MSYS2/MinGW dependencies for Qt applications
$knownDependencies = @(
    "libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll",
    "libdouble-conversion.dll", "libicudt77.dll", "libicuin77.dll", "libicuuc77.dll",
    "libpcre2-16-0.dll", "libzstd.dll", "libb2-1.dll",
    "libfreetype-6.dll", "libharfbuzz-0.dll", "libpng16-16.dll", "libjpeg-8.dll",
    "libmd4c.dll", "libgraphite2.dll", "libglib-2.0-0.dll", "libintl-8.dll",
    "libiconv-2.dll", "libpcre2-8-0.dll", "libbz2-1.dll", "libbrotlidec.dll",
    "libbrotlicommon.dll", "zlib1.dll", "libssl-3-x64.dll", "libcrypto-3-x64.dll"
)

$copiedCount = 0
foreach ($dll in $knownDependencies) {
    if ($dll -notin $existingDlls) {
        $sourcePath = Join-Path $msys64Bin $dll
        if (Test-Path $sourcePath) {
            Copy-Item $sourcePath $targetDir -Force
            Write-Host "  ✓ Copied: $dll" -ForegroundColor Green
            $copiedCount++
        }
    }
}

# Count total DLLs
$totalDlls = (Get-ChildItem "$targetDir\*.dll" -File).Count

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor White
Write-Host "  New dependencies copied: $copiedCount" -ForegroundColor Green
Write-Host "  Total DLLs in build: $totalDlls" -ForegroundColor White
Write-Host "`n✓ Deployment Complete!" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan
