# Applications CMake Configuration

# Application source files
set(APPLICATIONS_SOURCES
    # Placeholder - will be populated in future tasks
    applications_stub.c
)

# Create applications library
add_library(applications STATIC ${APPLICATIONS_SOURCES})

target_include_directories(applications PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Applications-specific compiler flags
target_compile_definitions(applications PRIVATE
    APPLICATIONS_LAYER=1
)

# Link with runtime, HAL, and kernel
target_link_libraries(applications runtime hal kernel)