#!/bin/bash
# Build script for TweaknGeek Firmware with ARM GCC

set -e  # Exit on any error

echo "=== TweaknGeek Firmware Build Script ==="

# Check if ARM GCC toolchain is available
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "ERROR: ARM GCC toolchain not found!"
    echo "Please install arm-none-eabi-gcc toolchain:"
    echo "  Ubuntu/Debian: sudo apt-get install gcc-arm-none-eabi"
    echo "  Arch: sudo pacman -S arm-none-eabi-gcc"
    echo "  Or download from: https://developer.arm.com/downloads/-/gnu-rm"
    exit 1
fi

echo "ARM GCC Toolchain found:"
arm-none-eabi-gcc --version | head -1

# Clean and create build directory
echo "Cleaning build directory..."
rm -rf build
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -G "Unix Makefiles"

# Build the firmware
echo "Building firmware..."
make -j$(nproc)

echo "=== Build completed successfully! ==="
echo "Firmware files:"
ls -la *.elf *.bin *.hex 2>/dev/null || echo "Binary files will be generated after successful build"