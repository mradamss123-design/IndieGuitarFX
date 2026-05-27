@echo off
setlocal EnableDelayedExpansion
title IndieGuitarFX — Windows Build Setup
color 0A

echo.
echo  ==========================================
echo   IndieGuitarFX VST3 — Windows Build Setup
echo  ==========================================
echo.

REM ---- Check for CMake ----
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found!
    echo Please download from: https://cmake.org/download/
    echo Make sure to select "Add CMake to system PATH"
    pause
    exit /b 1
)

echo [OK] CMake found:
cmake --version | findstr "cmake version"

REM ---- Check for Visual Studio 2022 ----
set "VS2022_PATH="
for %%p in (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist "%%~p" set "VS2022_PATH=%%~p"
)

if "!VS2022_PATH!"=="" (
    echo [ERROR] Visual Studio 2022 not found!
    echo Please install from: https://visualstudio.microsoft.com/
    echo Required workload: "Desktop development with C++"
    pause
    exit /b 1
)
echo [OK] Visual Studio 2022 found

REM ---- Get script directory ----
set "PROJ_DIR=%~dp0"
if "%PROJ_DIR:~-1%"=="\" set "PROJ_DIR=%PROJ_DIR:~0,-1%"

echo.
echo [INFO] Project directory: %PROJ_DIR%

REM ---- Find JUCE ----
set "JUCE_PATH="

REM Common JUCE locations
for %%p in (
    "C:\JUCE"
    "C:\Program Files\JUCE"
    "%USERPROFILE%\JUCE"
    "%USERPROFILE%\Documents\JUCE"
    "C:\SDKs\JUCE"
    "%PROJ_DIR%\..\JUCE"
) do (
    if exist "%%~p\CMakeLists.txt" set "JUCE_PATH=%%~p"
)

if "!JUCE_PATH!"=="" (
    echo.
    echo [INFO] JUCE not found in standard locations.
    echo.
    echo Choose an option:
    echo   1. Download JUCE automatically via Git
    echo   2. Enter path to existing JUCE installation
    echo.
    set /p CHOICE="Enter 1 or 2: "

    if "!CHOICE!"=="1" (
        where git >nul 2>&1
        if !ERRORLEVEL! NEQ 0 (
            echo [ERROR] Git not found. Install from https://git-scm.com/
            pause
            exit /b 1
        )
        echo.
        echo [INFO] Cloning JUCE 7.0.9 into C:\JUCE ...
        git clone --depth=1 --branch 7.0.9 https://github.com/juce-framework/JUCE.git C:\JUCE
        if !ERRORLEVEL! NEQ 0 (
            echo [ERROR] Git clone failed.
            pause
            exit /b 1
        )
        set "JUCE_PATH=C:\JUCE"
    ) else (
        set /p JUCE_PATH="Enter full path to JUCE: "
        if not exist "!JUCE_PATH!\CMakeLists.txt" (
            echo [ERROR] Invalid JUCE path — CMakeLists.txt not found there.
            pause
            exit /b 1
        )
    )
)

echo [OK] JUCE found at: !JUCE_PATH!

REM ---- Create build directory ----
set "BUILD_DIR=%PROJ_DIR%\build_vs2022"
if not exist "!BUILD_DIR!" mkdir "!BUILD_DIR!"

echo.
echo [INFO] Configuring project with CMake...
echo        This may take a minute while CMake processes JUCE...
echo.

cmake -S "%PROJ_DIR%" -B "!BUILD_DIR!" ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_PREFIX_PATH="!JUCE_PATH!" ^
    -DJUCE_DIR="!JUCE_PATH!\extras\Build\CMake"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    echo Check the error above.
    echo.
    echo TIP: If JUCE was not found, edit CMakeLists.txt and replace
    echo      the FetchContent block with:
    echo      add_subdirectory(!JUCE_PATH! juce_build)
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Project configured!
echo.

REM ---- Ask to build now ----
set /p BUILD_NOW="Build now in Release mode? (Y/N): "
if /i "!BUILD_NOW!"=="Y" (
    echo.
    echo [INFO] Building... (this may take 3-10 minutes)
    cmake --build "!BUILD_DIR!" --config Release --parallel

    if !ERRORLEVEL! NEQ 0 (
        echo.
        echo [ERROR] Build failed! Check errors above.
        pause
        exit /b 1
    )

    echo.
    echo [SUCCESS] Build complete!
    echo.

    REM ---- Install VST3 ----
    set "VST3_SRC=!BUILD_DIR!\IndieGuitarFX_artefacts\Release\VST3\IndieGuitarFX.vst3"
    set "VST3_DST=C:\Program Files\Common Files\VST3\IndieGuitarFX.vst3"

    if exist "!VST3_SRC!" (
        set /p INSTALL="Install VST3 to C:\Program Files\Common Files\VST3\ ? (Y/N): "
        if /i "!INSTALL!"=="Y" (
            echo [INFO] Copying VST3...
            xcopy "!VST3_SRC!" "!VST3_DST!\" /E /I /Y
            if !ERRORLEVEL! EQU 0 (
                echo [SUCCESS] VST3 installed! Restart FL Studio and scan for new plugins.
            ) else (
                echo [WARN] Copy failed — try running this script as Administrator.
                echo Manual path: !VST3_SRC!
            )
        )
    ) else (
        echo [WARN] VST3 not found at expected path:
        echo !VST3_SRC!
        echo Check the build output directory.
    )
) else (
    echo.
    echo [INFO] To open in Visual Studio:
    echo        !BUILD_DIR!\IndieGuitarFX.sln
    echo.
    echo [INFO] To build from command line:
    echo        cmake --build "!BUILD_DIR!" --config Release
)

echo.
echo ========================================
echo  Done! See BUILD_INSTRUCTIONS.md for
echo  detailed usage and preset guide.
echo ========================================
echo.
pause
