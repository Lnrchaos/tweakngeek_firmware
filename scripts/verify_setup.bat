@echo off
REM TweaknGeek Firmware Setup Verification Script for Windows
REM This script verifies that the project structure and build system are properly configured

echo TweaknGeek Firmware Setup Verification
echo ======================================

echo Checking project structure...

REM Check directories
set DIRS=src\kernel src\hal src\runtime src\applications src\services include build scripts linker cmake

for %%d in (%DIRS%) do (
    if exist "%%d" (
        echo ✓ Directory: %%d
    ) else (
        echo ✗ Missing directory: %%d
    )
)

REM Check files
set FILES=CMakeLists.txt src\main.c cmake\arm-none-eabi-gcc.cmake linker\STM32WB55RG_FLASH.ld include\tweakngeek_config.h

for %%f in (%FILES%) do (
    if exist "%%f" (
        echo ✓ File: %%f
    ) else (
        echo ✗ Missing file: %%f
    )
)

echo.
echo Checking CMake configuration...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ✗ CMake not found
) else (
    echo ✓ CMake is available
    cmake --version | findstr /C:"cmake version"
)

echo.
echo Checking ARM toolchain...
arm-none-eabi-gcc --version >nul 2>&1
if errorlevel 1 (
    echo ✗ ARM GCC toolchain not found
    echo   Download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
) else (
    echo ✓ ARM GCC toolchain is available
    arm-none-eabi-gcc --version | findstr /C:"arm-none-eabi-gcc"
)

echo.
echo Setup verification complete!
echo.
echo To build the firmware:
echo   scripts\build.bat Debug    # Debug build
echo   scripts\build.bat Release  # Release build