set(PROJECT_NAME lib_generic_c)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "../aes.h"
    "../aes_ni.h"
    "../aes_via_ace.h"
    "../aesopt.h"
    "../aestab.h"
    "../brg_endian.h"
    "../brg_types.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "../aes_modes.c"
    "../aes_ni.c"
    "../aescrypt.c"
    "../aeskey.c"
    "../aestab.c"
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

set(ROOT_NAMESPACE lib_generic_c)

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
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION         "FALSE"
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
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "WIN32;"
    "_CONSOLE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Gm;
            /Od;
            /RTC1;
            /W4
        >
        $<$<CONFIG:Release>:
            /GL-;
            /Ox;
            /Ot
        >
        /Zi;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/Od"
        ">"
        "$<$<CONFIG:Release>:"
            "/O1"
        ">"
    )
    source_file_compile_options(../aescrypt.c ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Release>:"
            "/O1"
        ">"
    )
    source_file_compile_options(../aeskey.c ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Release>:"
            "/O1"
        ">"
    )
    source_file_compile_options(../aestab.c ${FILE_CL_OPTIONS})
endif()

