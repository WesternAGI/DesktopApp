# Why DLLs Aren't Automatically Copied During Build

## The Problem

When building Qt applications with CMake/MinGW, **only the executable is built**. The required DLLs are **NOT automatically copied** because:

### 1. **CMake doesn't copy runtime dependencies**
   - CMake only compiles and links your code
   - It creates the `.exe` but doesn't deploy DLLs
   - DLLs remain in their original locations (MSYS2/MinGW directories)

### 2. **windeployqt6 only copies Qt DLLs**
   - It copies Qt's own DLLs (Qt6Core, Qt6Gui, etc.)
   - It copies Qt plugins (platforms, imageformats, etc.)
   - **It does NOT copy system/MinGW dependencies** like:
     - libgcc_s_seh-1.dll (GCC runtime)
     - libstdc++-6.dll (C++ standard library)
     - libwinpthread-1.dll (threading)
     - libfreetype, libharfbuzz, libpng (font/image libraries)
     - etc.

### 3. **Transitive dependencies are not detected**
   - Qt DLLs themselves depend on other DLLs
   - Example chain:
     ```
     DesktopApp.exe
       └─ Qt6Gui.dll
            └─ libfreetype-6.dll
                 └─ libharfbuzz-0.dll
                      └─ libglib-2.0-0.dll
                           └─ libintl-8.dll
                                └─ libiconv-2.dll
     ```
   - windeployqt6 doesn't follow these chains

## Solutions

### Solution 1: Add MSYS2 to PATH (Development)
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
.\build\DesktopApp.exe
```
**Pros:** Simple, no copying needed
**Cons:** Not portable, requires MSYS2 on every machine

### Solution 2: Deploy ALL Dependencies (Production)
```powershell
# Use our smart deployment script
powershell -ExecutionPolicy Bypass -File .\deploy-deps.ps1
```
**Pros:** Standalone, portable
**Cons:** Larger package size

### Solution 3: Use windeployqt6 + Manual Copy
```powershell
# Step 1: Deploy Qt DLLs
windeployqt6 --no-translations .\build\DesktopApp.exe

# Step 2: Copy MinGW runtime
Copy-Item C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll .\build\
Copy-Item C:\msys64\ucrt64\bin\libstdc++-6.dll .\build\
Copy-Item C:\msys64\ucrt64\bin\libwinpthread-1.dll .\build\
# ... and all other dependencies
```
**Pros:** Full control
**Cons:** Manual, error-prone

## What We've Done

We created **three deployment scripts**:

1. **`deploy-dlls.ps1`** - Original script with manual DLL list
2. **`deploy-all-dlls.ps1`** - Comprehensive but copies too many (~427 DLLs)
3. **`deploy-deps.ps1`** - Smart script that copies only known dependencies (~50 DLLs)

## Recommended Workflow

### For Development:
```powershell
# Use the launcher script
.\run-app.ps1
```

### For Distribution:
```powershell
# Build in Release mode
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release

# Deploy all dependencies
powershell -ExecutionPolicy Bypass -File .\deploy-deps.ps1 -BuildDir build-release

# Package the build-release directory
```

## Understanding the DLL Dependency Tree

Our application's actual dependencies:

```
DesktopApp.exe (24 MB)
├─ MinGW Runtime
│  ├─ libgcc_s_seh-1.dll
│  ├─ libstdc++-6.dll
│  └─ libwinpthread-1.dll
│
├─ Qt6 Framework (~38 MB)
│  ├─ Qt6Core.dll
│  │  ├─ libdouble-conversion.dll
│  │  ├─ libicuXX.dll (3 files)
│  │  ├─ libpcre2-16-0.dll
│  │  ├─ libzstd.dll
│  │  └─ libb2-1.dll
│  │
│  ├─ Qt6Gui.dll
│  │  ├─ libfreetype-6.dll
│  │  │  ├─ libbz2-1.dll
│  │  │  ├─ libpng16-16.dll
│  │  │  ├─ libbrotli*.dll
│  │  │  └─ zlib1.dll
│  │  ├─ libharfbuzz-0.dll
│  │  │  ├─ libgraphite2.dll
│  │  │  └─ libglib-2.0-0.dll
│  │  │     ├─ libintl-8.dll
│  │  │     ├─ libiconv-2.dll
│  │  │     └─ libpcre2-8-0.dll
│  │  ├─ libmd4c.dll
│  │  └─ libjpeg-8.dll
│  │
│  ├─ Qt6Widgets.dll
│  ├─ Qt6Network.dll
│  │  ├─ libssl-3-x64.dll
│  │  └─ libcrypto-3-x64.dll
│  ├─ Qt6Multimedia.dll
│  └─ Qt6Svg.dll
│
└─ FFmpeg Libraries (~26 MB)
   ├─ avcodec-62.dll
   ├─ avformat-62.dll
   ├─ avutil-60.dll
   ├─ swresample-6.dll
   └─ swscale-9.dll
```

**Total: ~50-60 essential DLLs, ~100 MB total**

## Why This Matters

- **Development**: You can use PATH to avoid copying (fast iteration)
- **Testing**: Copy DLLs to test standalone deployment
- **Production**: Always deploy ALL dependencies for distribution
- **Size**: Essential DLLs only = ~100 MB vs. All MSYS2 DLLs = ~500+ MB

## Conclusion

**DLLs are not copied during build by design.** It's your responsibility to:
1. Either add the DLL location to PATH, OR
2. Explicitly deploy all dependencies to the build directory

Use our provided scripts to automate this process!
