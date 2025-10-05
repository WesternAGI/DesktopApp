# DLL Deployment Solution - Quick Reference

## Problem
When running `DesktopApp.exe`, Windows reported missing DLLs:
- Qt6Core.dll
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- Qt6Gui.dll

## Solution Implemented

### 1. Automatic Deployment (Recommended)
Use Qt's `windeployqt6` tool to automatically copy all required DLLs and plugins:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
windeployqt6 --no-translations .\build\DesktopApp.exe
```

Or use the provided script:
```powershell
powershell -ExecutionPolicy Bypass -File .\deploy-dlls.ps1
```

### 2. Launcher Script (For Development)
Use the `run-app.ps1` script which automatically sets up the PATH:

```powershell
powershell -ExecutionPolicy Bypass -File .\run-app.ps1
```

### 3. Manual PATH Setup
Add MSYS2 UCRT64 to PATH before running:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
.\build\DesktopApp.exe
```

## What Was Deployed

The following were copied to the `build/` directory:

**Core DLLs:**
- Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll
- Qt6Network.dll, Qt6Multimedia.dll, Qt6Svg.dll, Qt6Pdf.dll
- libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll (MinGW runtime)
- avcodec, avformat, avutil, swresample, swscale (FFmpeg for multimedia)

**Qt Plugins:**
- platforms/ (qwindows.dll - Windows platform integration)
- styles/ (qmodernwindowsstyle.dll)
- imageformats/ (qjpeg.dll, qpng.dll, qsvg.dll, etc.)
- iconengines/ (qsvgicon.dll)
- multimedia/ (ffmpegmediaplugin.dll)
- networkinformation/ (network backend plugins)
- tls/ (SSL/TLS backend plugins)

## Files Created

1. **run-app.ps1** - Launcher script that sets up PATH and runs the app
2. **deploy-dlls.ps1** - Script to deploy all DLLs using windeployqt6
3. **build/README.txt** - Documentation in the build directory
4. **BUILD_NOTES.md** - Updated with DLL deployment instructions

## Current Status

✅ All required DLLs deployed to `build/` directory
✅ Application runs standalone without PATH modifications
✅ Can be redistributed by copying the entire `build/` directory
✅ Scripts committed and pushed to repository

## Testing

To verify the deployment works:

```powershell
cd C:\Users\gad\Desktop\DesktopApp\build
.\DesktopApp.exe --version
```

This should run without any DLL errors.

## For Distribution

To create a distributable package:
1. Build in Release mode: `cmake --build build-release --config Release`
2. Deploy DLLs: `windeployqt6 --no-translations .\build-release\DesktopApp.exe`
3. Copy the entire `build-release/` directory
4. Optionally create an installer using CPack

## Commit Summary

- Commit: 492fa66
- Message: "Add DLL deployment scripts and fix runtime dependencies"
- Files: deploy-dlls.ps1, run-app.ps1, BUILD_NOTES.md
- Pushed to: origin/main
