@echo off
REM ============================================================================
REM OpenWare OS - Windows Build Wrapper for WSL
REM Copyright (c) 2026 Ventryx Inc. All rights reserved.
REM ============================================================================

setlocal enabledelayedexpansion

REM Check if WSL is available
where wsl >nul 2>&1
if errorlevel 1 (
    echo ERROR: WSL is not installed or not in PATH.
    echo.
    echo Install WSL with:
    echo   wsl --install
    echo.
    pause
    exit /b 1
)

REM Get the current directory in WSL path format
set "WIN_PATH=%~dp0"
set "WIN_PATH=%WIN_PATH:~0,-1%"

REM Convert Windows path to WSL path
for /f "delims=" %%i in ('wsl wslpath -u "%WIN_PATH%"') do set "WSL_PATH=%%i"

echo ====================================================
echo  OpenWare OS - WSL Build System
echo ====================================================
echo.
echo Windows Path: %WIN_PATH%
echo WSL Path:     %WSL_PATH%
echo.

REM Determine command
set "CMD=all"
if not "%1"=="" set "CMD=%1"

REM Run the build script in WSL
echo Running: ./build_wsl.sh %CMD%
echo.
wsl -e bash -c "cd '%WSL_PATH%' && chmod +x build_wsl.sh && ./build_wsl.sh %CMD%"

if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ====================================================
echo  Build Complete!
echo ====================================================

if "%CMD%"=="all" (
    echo.
    echo To run in QEMU:
    echo   build.bat run
    echo.
    echo Or manually:
    echo   qemu-system-i386 -cdrom build\openware.iso
)

pause
