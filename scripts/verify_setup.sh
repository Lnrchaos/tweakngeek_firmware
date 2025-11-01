#!/bin/bash

# TweaknGeek Firmware Setup Verification Script
# This script verifies that the project structure and build system are properly configured

echo "TweaknGeek Firmware Setup Verification"
echo "======================================"

# Check project structure
echo "Checking project structure..."

REQUIRED_DIRS=(
    "src/kernel"
    "src/hal" 
    "src/runtime"
    "src/applications"
    "src/services"
    "include"
    "build"
    "scripts"
    "linker"
    "cmake"
)

REQUIRED_FILES=(
    "CMakeLists.txt"
    "src/main.c"
    "cmake/arm-none-eabi-gcc.cmake"
    "linker/STM32WB55RG_FLASH.ld"
    "include/tweakngeek_config.h"
)

# Check directories
for dir in "${REQUIRED_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo "✓ Directory: $dir"
    else
        echo "✗ Missing directory: $dir"
    fi
done

# Check files
for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ File: $file"
    else
        echo "✗ Missing file: $file"
    fi
done

# Check CMake configuration
echo ""
echo "Checking CMake configuration..."
if command -v cmake &> /dev/null; then
    echo "✓ CMake is available"
    cmake --version | head -1
else
    echo "✗ CMake not found"
fi

# Check for ARM toolchain
echo ""
echo "Checking ARM toolchain..."
if command -v arm-none-eabi-gcc &> /dev/null; then
    echo "✓ ARM GCC toolchain is available"
    arm-none-eabi-gcc --version | head -1
else
    echo "✗ ARM GCC toolchain not found"
    echo "  Install with: sudo apt install gcc-arm-none-eabi (Ubuntu/Debian)"
    echo "  Or download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm"
fi

echo ""
echo "Setup verification complete!"
echo ""
echo "To build the firmware:"
echo "  ./scripts/build.sh Debug    # Debug build"
echo "  ./scripts/build.sh Release  # Release build"