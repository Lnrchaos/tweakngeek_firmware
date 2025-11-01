# Kernel Layer CMake Configuration

# Kernel source files
set(KERNEL_SOURCES
    kernel_core.c
    boot.c
    startup_stm32wb55.s
    memory.c
    scheduler.c
    interrupt.c
    interrupt_handlers.c
    syscalls.c
)

# Create kernel library
add_library(kernel STATIC ${KERNEL_SOURCES})

target_include_directories(kernel PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Kernel-specific compiler flags
target_compile_definitions(kernel PRIVATE
    KERNEL_LAYER=1
)

# Place kernel code in special section
target_compile_options(kernel PRIVATE
    -ffunction-sections
    -fdata-sections
)