# System Services CMake Configuration

# Services source files
set(SERVICES_SOURCES
    # Placeholder - will be populated in future tasks
    services_stub.c
)

# Create services library
add_library(services STATIC ${SERVICES_SOURCES})

target_include_directories(services PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Services-specific compiler flags
target_compile_definitions(services PRIVATE
    SERVICES_LAYER=1
)

# Link with HAL and kernel
target_link_libraries(services hal kernel)