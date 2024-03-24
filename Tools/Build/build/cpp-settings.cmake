# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMPILE_OPTIONS "")
set(LINK_OPTIONS "")

if (WIN32)

    # Disable secure CRT warnings on windows, our code is designed to be platform agnostic
    # and these get in the way of that.
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -D_CRT_SECURE_NO_WARNINGS)

    # Platform type define.
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -DDS_PLATFORM_WINDOWS)

elseif (UNIX)

    # Platform type define.
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -DDS_PLATFORM_LINUX)
    
    # Use permissive flags, GCC/clang are stricter that MSVC.
    # We should go in and fix up the problematic areas and remove this later.
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -g")

    # Use dynamic linker flag so we can lookup symbols at runtime.
    # Also enforce symbolic information in all builds.
    set(LINK_OPTIONS ${LINK_OPTIONS} -rdynamic -g)
    
endif()

# Architecture define.
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -DDS_PLATFORM_X86)
else()
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -DDS_PLATFORM_X64)
endif()

if (WIN32)
    # We always want to generate debug information in all configurations.
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /Zi)
    set(LINK_OPTIONS ${LINK_OPTIONS} /DEBUG)
endif()

set(DEBUG_COMPILE_OPTIONS   ${COMPILE_OPTIONS} -DDS_CONFIG_DEBUG)
set(RELEASE_COMPILE_OPTIONS ${COMPILE_OPTIONS} -DDS_CONFIG_RELEASE)

set(DEBUG_LINK_OPTIONS   ${LINK_OPTIONS})
set(RELEASE_LINK_OPTIONS ${LINK_OPTIONS})

add_compile_options(
    "$<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>"
    "$<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>"
)

add_link_options(
    "$<$<CONFIG:Debug>:${DEBUG_LINK_OPTIONS}>"
    "$<$<CONFIG:Release>:${RELEASE_LINK_OPTIONS}>"
)
