@echo off
REM Build script for TweaknGeek Firmware with ARM GCC

echo Setting up ARM GCC environment...
set "ARM_TOOLCHAIN_PATH=C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin"
set "PATH=%ARM_TOOLCHAIN_PATH%;%PATH%"

echo Checking ARM GCC installation...
arm-none-eabi-gcc --version
if %ERRORLEVEL% neq 0 (
    echo ERROR: ARM GCC not found in PATH
    pause
    exit /b 1
)

echo Cleaning build directory...
if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with ARM GCC toolchain...
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake -DCMAKE_BUILD_TYPE=Debug

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo Building firmware...
cmake --build . --config Debug

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo Build completed successfully!
pause