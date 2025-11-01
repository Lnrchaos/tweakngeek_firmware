# Application Runtime CMake Configuration

# Runtime source files
set(RUNTIME_SOURCES
    # Placeholder - will be populated in future tasks
    runtime_stub.c
)

# Create runtime library
add_library(runtime STATIC ${RUNTIME_SOURCES})

target_include_directories(runtime PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Runtime-specific compiler flags
target_compile_definitions(runtime PRIVATE
    RUNTIME_LAYER=1
)

# Link with kernel and HAL
target_link_libraries(runtime kernel hal)