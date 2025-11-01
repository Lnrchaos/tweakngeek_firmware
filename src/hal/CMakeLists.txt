# Hardware Abstraction Layer CMake Configuration

# HAL source files
set(HAL_SOURCES
    # Placeholder - will be populated in future tasks
    hal_stub.c
)

# Create HAL library
add_library(hal STATIC ${HAL_SOURCES})

target_include_directories(hal PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# HAL-specific compiler flags
target_compile_definitions(hal PRIVATE
    HAL_LAYER=1
)

# Link with kernel for hardware access
target_link_libraries(hal kernel)