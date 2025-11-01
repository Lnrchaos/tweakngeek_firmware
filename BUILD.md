# TweaknGeek Firmware Build Instructions

This document provides instructions for building the TweaknGeek firmware from source.

## Prerequisites

### Required Tools

1. **ARM GCC Toolchain** (arm-none-eabi-gcc)
   - Ubuntu/Debian: `sudo apt install gcc-arm-none-eabi`
   - macOS: `brew install arm-none-eabi-gcc`
   - Windows: Download from [ARM Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

2. **CMake** (version 3.16 or later)
   - Ubuntu/Debian: `sudo apt install cmake`
   - macOS: `brew install cmake`
   - Windows: Download from [CMake.org](https://cmake.org/download/)

3. **Make** (or Ninja)
   - Ubuntu/Debian: `sudo apt install make`
   - macOS: Included with Xcode Command Line Tools
   - Windows: Use MinGW or MSYS2

### Verification

Verify your toolchain installation:

```bash
arm-none-eabi-gcc --version
cmake --version
make --version
```

## Building the Firmware

### Quick Build (Linux/macOS)

```bash
# Make build script executable
chmod +x scripts/build.sh

# Build debug version
./scripts/build.sh Debug

# Build release version
./scripts/build.sh Release
```

### Quick Build (Windows)

```cmd
REM Build debug version
scripts\build.bat Debug

REM Build release version
scripts\build.bat Release
```

### Manual Build

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake

# Build firmware
make -j4

# Or use CMake build command
cmake --build . --parallel
```

## Build Outputs

After successful build, you will find the following files in the `build/` directory:

- `tweakngeek_firmware.elf` - ELF executable with debug symbols
- `tweakngeek.bin` - Raw binary firmware image
- `tweakngeek.hex` - Intel HEX format firmware
- `tweakngeek.map` - Memory map file

## Build Configuration

### Build Types

- **Debug**: Includes debug symbols, no optimization (`-g3 -O0`)
- **Release**: Optimized for size (`-Os`), no debug symbols

### Memory Layout

The firmware uses the following memory layout for STM32WB55RG:

- **Flash**: 1MB starting at 0x08000000
- **SRAM1**: 192KB starting at 0x20000000 (main system memory)
- **SRAM2A**: 32KB starting at 0x20030000 (application memory)
- **SRAM2B**: 32KB starting at 0x20038000 (system reserved)

### Customization

Edit `include/tweakngeek_config.h` to customize:

- Memory allocation sizes
- Feature flags
- Hardware configuration
- Debug settings

## Troubleshooting

### Common Issues

1. **Toolchain not found**
   - Ensure ARM GCC toolchain is installed and in PATH
   - Verify with `arm-none-eabi-gcc --version`

2. **CMake configuration fails**
   - Check CMake version (3.16+ required)
   - Verify toolchain file path is correct

3. **Build fails with memory errors**
   - Check linker script memory regions
   - Verify code size fits in flash memory

4. **Windows-specific issues**
   - Use MinGW or MSYS2 for make
   - Ensure proper path separators in scripts

### Getting Help

If you encounter issues:

1. Check the build log for specific error messages
2. Verify all prerequisites are installed correctly
3. Try a clean build: `rm -rf build && mkdir build`
4. Check that your system meets the minimum requirements

## Next Steps

After successful build:

1. Flash the firmware to your FlipperZero device
2. Verify boot sequence and TweaknGeek splash screen
3. Test basic functionality
4. Proceed with implementing additional features

For flashing instructions, see the main README.md file.