#!/bin/bash

# TweaknGeek Firmware Build Script
# This script sets up the build environment and compiles the firmware

set -e

# Configuration
BUILD_TYPE=${1:-Debug}
BUILD_DIR="build"
TOOLCHAIN_FILE="cmake/arm-none-eabi-gcc.cmake"

echo "Building TweaknGeek Firmware..."
echo "Build type: $BUILD_TYPE"

# Check if ARM GCC toolchain is available
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "Error: ARM GCC toolchain not found!"
    echo "Please install arm-none-eabi-gcc toolchain:"
    echo "  Ubuntu/Debian: sudo apt install gcc-arm-none-eabi"
    echo "  macOS: brew install arm-none-eabi-gcc"
    echo "  Windows: Install from ARM developer website"
    exit 1
fi

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
echo "Configuring build system..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=../$TOOLCHAIN_FILE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build firmware
echo "Building firmware..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build completed successfully!"
echo "Firmware files:"
echo "  Binary: $BUILD_DIR/tweakngeek.bin"
echo "  Hex:    $BUILD_DIR/tweakngeek.hex"
echo "  ELF:    $BUILD_DIR/tweakngeek_firmware.elf"