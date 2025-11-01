# TweaknGeek Firmware Project Structure

This document describes the organization and structure of the TweaknGeek firmware project.

## Directory Structure

```
TweaknGeek_Firmware/
├── .kiro/                          # Kiro IDE configuration
│   └── specs/                      # Feature specifications
│       └── tweakngeek-firmware/    # This project's specs
├── build/                          # Build artifacts (generated)
├── cmake/                          # CMake configuration files
│   └── arm-none-eabi-gcc.cmake    # Cross-compilation toolchain
├── include/                        # Public header files
│   └── tweakngeek_config.h        # System configuration
├── linker/                         # Linker scripts
│   └── STM32WB55RG_FLASH.ld       # Memory layout for STM32WB55
├── scripts/                        # Build and utility scripts
│   ├── build.sh                   # Linux/macOS build script
│   ├── build.bat                  # Windows build script
│   ├── verify_setup.sh            # Linux/macOS setup verification
│   └── verify_setup.bat           # Windows setup verification
└── src/                           # Source code
    ├── main.c                     # Main entry point
    ├── kernel/                    # Kernel layer
    │   ├── CMakeLists.txt
    │   ├── kernel_stub.c          # Placeholder implementation
    │   └── README.md
    ├── hal/                       # Hardware Abstraction Layer
    │   ├── CMakeLists.txt
    │   ├── hal_stub.c             # Placeholder implementation
    │   └── README.md
    ├── runtime/                   # Application Runtime
    │   ├── CMakeLists.txt
    │   ├── runtime_stub.c         # Placeholder implementation
    │   └── README.md
    ├── applications/              # Built-in Applications
    │   ├── CMakeLists.txt
    │   ├── applications_stub.c    # Placeholder implementation
    │   └── README.md
    └── services/                  # System Services
        ├── CMakeLists.txt
        ├── services_stub.c        # Placeholder implementation
        └── README.md
```

## Architecture Layers

### 1. Kernel Layer (`src/kernel/`)
- **Purpose**: Core system management and hardware control
- **Components**: Process scheduling, memory management, interrupt handling, system calls
- **Status**: Stub implementation (Task 2)

### 2. Hardware Abstraction Layer (`src/hal/`)
- **Purpose**: Standardized interfaces to hardware components
- **Components**: GPIO HAL, Radio HAL, Display HAL, Storage HAL
- **Status**: Stub implementation (Task 3)

### 3. Application Runtime (`src/runtime/`)
- **Purpose**: Execute FlipperZero applications with compatibility
- **Components**: Application loader, API compatibility, sandboxing, lifecycle management
- **Status**: Stub implementation (Task 4)

### 4. System Services (`src/services/`)
- **Purpose**: System-level background services
- **Components**: Power management, security, monitoring, diagnostics
- **Status**: Stub implementation (Task 7)

### 5. Applications (`src/applications/`)
- **Purpose**: Built-in system applications and advanced features
- **Components**: WiFi emulation, custom layout engine, splash screen
- **Status**: Stub implementation (Tasks 5-8)

## Build System

### CMake Configuration
- **Main**: `CMakeLists.txt` - Root build configuration
- **Toolchain**: `cmake/arm-none-eabi-gcc.cmake` - Cross-compilation setup
- **Subdirectories**: Each layer has its own `CMakeLists.txt`

### Target Hardware
- **MCU**: STM32WB55RG (ARM Cortex-M4)
- **Platform**: FlipperZero device
- **Memory**: 1MB Flash, 256KB SRAM
- **Linker Script**: `linker/STM32WB55RG_FLASH.ld`

### Build Outputs
- `tweakngeek_firmware.elf` - ELF executable with debug symbols
- `tweakngeek.bin` - Raw binary firmware image
- `tweakngeek.hex` - Intel HEX format firmware
- `tweakngeek.map` - Memory map file

## Configuration

### System Configuration (`include/tweakngeek_config.h`)
- Firmware version information
- Hardware configuration constants
- Memory allocation settings
- Feature flags and debug options

### Memory Layout
- **Flash**: Kernel, HAL, Runtime, Applications code
- **SRAM1**: Main system memory (heap, stack, globals)
- **SRAM2A**: Application memory pool
- **SRAM2B**: System reserved memory

## Development Workflow

### Current Status
✅ **Task 1 Complete**: Project structure and build system set up
- Directory structure created
- CMake build system configured
- Cross-compilation toolchain setup
- Linker scripts for memory layout
- Build scripts and verification tools

### Next Steps
- **Task 2**: Implement core kernel foundation
- **Task 3**: Develop hardware abstraction layer
- **Task 4**: Create application runtime system
- **Task 5**: Develop WiFi emulation service
- **Task 6**: Implement custom layout engine
- **Task 7**: Develop system services and utilities
- **Task 8**: Implement splash screen and branding
- **Task 9**: Integration and system testing
- **Task 10**: Documentation and deployment preparation

### Building
```bash
# Verify setup
./scripts/verify_setup.sh    # Linux/macOS
scripts\verify_setup.bat     # Windows

# Build firmware
./scripts/build.sh Debug     # Linux/macOS
scripts\build.bat Debug      # Windows
```

## Requirements Mapping

This project structure addresses the following requirements:

- **Requirement 2.1**: Custom kernel implementation (kernel layer)
- **Requirement 2.2**: Hardware abstraction layer (HAL layer)
- **Requirement 3.1**: Application runtime compatibility (runtime layer)
- **Requirement 5.1**: WiFi emulation service (applications layer)
- **Requirement 4.1**: Custom layout engine (applications layer)
- **Requirement 7.1**: System monitoring (services layer)

The modular architecture ensures clean separation of concerns and enables incremental development of each component.