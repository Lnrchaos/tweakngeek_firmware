@echo off
REM TweaknGeek Firmware Build Script for Windows
REM This script sets up the build environment and compiles the firmware

setlocal enabledelayedexpansion

REM Configuration
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug
set BUILD_DIR=build
set TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake

echo Building TweaknGeek Firmware...
echo Build type: %BUILD_TYPE%

REM Check if ARM GCC toolchain is available
arm-none-eabi-gcc --version >nul 2>&1
if errorlevel 1 (
    echo Error: ARM GCC toolchain not found!
    echo Please install arm-none-eabi-gcc toolchain
    echo Download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
    exit /b 1
)

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure with CMake
echo Configuring build system...
cmake .. ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE=../%TOOLCHAIN_FILE% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -G "MinGW Makefiles"

if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build firmware
echo Building firmware...
cmake --build . --parallel

if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Firmware files:
echo   Binary: %BUILD_DIR%/tweakngeek.bin
echo   Hex:    %BUILD_DIR%/tweakngeek.hex
echo   ELF:    %BUILD_DIR%/tweakngeek_firmware.elf