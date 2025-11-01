cmake_minimum_required(VERSION 3.16)

# Project configuration
project(TweaknGeek_Firmware
    VERSION 1.0.0
    DESCRIPTION "Custom firmware for FlipperZero device"
    LANGUAGES C CXX ASM
)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target configuration for STM32WB55
set(MCU_FAMILY STM32WB)
set(MCU_MODEL STM32WB55xx)

# Cross-compilation toolchain
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake)
endif()

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler flags for STM32WB55
set(MCU_FLAGS 
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# Common compiler flags
set(COMMON_FLAGS
    ${MCU_FLAGS}
    -Wall
    -Wextra
    -Wpedantic
    -fdata-sections
    -ffunction-sections
    -fstack-usage
    --specs=nano.specs
)

# Debug flags
set(DEBUG_FLAGS
    -g3
    -gdwarf-2
    -DDEBUG
    -O0
)

# Release flags
set(RELEASE_FLAGS
    -Os
    -DNDEBUG
)

# Apply flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${COMMON_FLAGS} ${DEBUG_FLAGS})
else()
    add_compile_options(${COMMON_FLAGS} ${RELEASE_FLAGS})
endif()

# Linker flags
set(LINKER_FLAGS
    ${MCU_FLAGS}
    -T${CMAKE_SOURCE_DIR}/linker/STM32WB55RG_FLASH.ld
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_BINARY_DIR}/tweakngeek.map
    --specs=nano.specs
)

add_link_options(${LINKER_FLAGS})

# Add subdirectories
add_subdirectory(src/kernel)
add_subdirectory(src/hal)
add_subdirectory(src/runtime)
add_subdirectory(src/applications)
add_subdirectory(src/services)

# Main firmware target
add_executable(tweakngeek_firmware
    src/main.c
)

target_link_libraries(tweakngeek_firmware
    kernel
    hal
    runtime
    applications
    services
)

# Generate binary and hex files
add_custom_command(TARGET tweakngeek_firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:tweakngeek_firmware> ${CMAKE_BINARY_DIR}/tweakngeek.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:tweakngeek_firmware> ${CMAKE_BINARY_DIR}/tweakngeek.hex
    COMMENT "Generating binary and hex files"
)

# Print size information
add_custom_command(TARGET tweakngeek_firmware POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:tweakngeek_firmware>
    COMMENT "Firmware size information"
)