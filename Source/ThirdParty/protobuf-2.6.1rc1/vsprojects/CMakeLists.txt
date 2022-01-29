set(PROJECT_NAME libprotobuf-lite)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "../src/google/protobuf/extension_set.h"
    "../src/google/protobuf/generated_message_util.h"
    "../src/google/protobuf/io/coded_stream.h"
    "../src/google/protobuf/io/coded_stream_inl.h"
    "../src/google/protobuf/io/zero_copy_stream.h"
    "../src/google/protobuf/io/zero_copy_stream_impl_lite.h"
    "../src/google/protobuf/message_lite.h"
    "../src/google/protobuf/repeated_field.h"
    "../src/google/protobuf/stubs/atomicops.h"
    "../src/google/protobuf/stubs/atomicops_internals_x86_msvc.h"
    "../src/google/protobuf/stubs/common.h"
    "../src/google/protobuf/stubs/hash.h"
    "../src/google/protobuf/stubs/once.h"
    "../src/google/protobuf/stubs/platform_macros.h"
    "../src/google/protobuf/stubs/stl_util.h"
    "../src/google/protobuf/stubs/stringprintf.h"
    "../src/google/protobuf/stubs/template_util.h"
    "../src/google/protobuf/stubs/type_traits.h"
    "../src/google/protobuf/wire_format_lite.h"
    "../src/google/protobuf/wire_format_lite_inl.h"
    "config.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "../src/google/protobuf/extension_set.cc"
    "../src/google/protobuf/generated_message_util.cc"
    "../src/google/protobuf/io/coded_stream.cc"
    "../src/google/protobuf/io/zero_copy_stream.cc"
    "../src/google/protobuf/io/zero_copy_stream_impl_lite.cc"
    "../src/google/protobuf/message_lite.cc"
    "../src/google/protobuf/repeated_field.cc"
    "../src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc"
    "../src/google/protobuf/stubs/common.cc"
    "../src/google/protobuf/stubs/once.cc"
    "../src/google/protobuf/stubs/stringprintf.cc"
    "../src/google/protobuf/wire_format_lite.cc"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "ThirdParty")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}//../Bin/$ENV{MSBuildProjectName}/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}//../Bin/$ENV{MSBuildProjectName}/"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebugDLL
    >
    $<$<CONFIG:Release>:
        MultiThreadedDLL
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/."
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS;"
    "WIN32;"
    "_WINDOWS;"
    "_USRDLL;"
    "LIBPROTOBUF_EXPORTS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /RTC1
        >
        /W3;
        /Zi;
        /wd4244;
        /wd4267;
        /wd4018;
        /wd4355;
        /wd4800;
        /wd4251;
        /wd4996;
        /wd4146;
        /wd4305;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
endif()

