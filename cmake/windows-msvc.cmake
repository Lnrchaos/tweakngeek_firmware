# Windows MSVC toolchain for development/testing
# This is NOT for the actual firmware - just for code validation

set(CMAKE_SYSTEM_NAME Windows)

# MSVC-compatible compiler flags
set(COMMON_FLAGS_MSVC
    /W4          # High warning level (equivalent to -Wall -Wextra)
    /WX-         # Don't treat warnings as errors
    /permissive- # Disable non-conforming code
)

# Debug flags for MSVC
set(DEBUG_FLAGS_MSVC
    /Od          # Disable optimization
    /Zi          # Generate debug info
    /DDEBUG
)

# Release flags for MSVC
set(RELEASE_FLAGS_MSVC
    /O2          # Optimize for speed
    /DNDEBUG
)

# Override the ARM-specific flags when using MSVC
if(MSVC)
    # Clear ARM-specific flags
    set(MCU_FLAGS "")
    set(COMMON_FLAGS ${COMMON_FLAGS_MSVC})
    set(DEBUG_FLAGS ${DEBUG_FLAGS_MSVC})
    set(RELEASE_FLAGS ${RELEASE_FLAGS_MSVC})
    set(LINKER_FLAGS "")
    
    # Disable ARM-specific post-build commands
    set(SKIP_ARM_POST_BUILD TRUE)
endif()