# Hardware Abstraction Layer CMake Configuration

# HAL source files
set(HAL_SOURCES
    hal_base.c
    hal_utils.c
    hal_gpio.c
    hal_radio.c
    hal_display.c
    hal_stub.c
)

# Create HAL library
add_library(hal STATIC ${HAL_SOURCES})

target_include_directories(hal PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(hal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# HAL-specific compiler flags
target_compile_definitions(hal PRIVATE
    HAL_LAYER=1
)

# Link with kernel for hardware access
target_link_libraries(hal kernel)