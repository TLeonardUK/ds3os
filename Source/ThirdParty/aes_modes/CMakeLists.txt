set(PROJECT_NAME aes_modes)

find_program(YASM_EXE NAMES yasm)
add_custom_command(OUTPUT aes_amd64.o COMMAND ${YASM_EXE}
        ARGS -DINTEL_AES_POSSIBLE=1 -f elf64 -o aes_amd64.o "${CMAKE_CURRENT_SOURCE_DIR}/../aes/aes_amd64.asm")
################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "../aes/aes_ni.c"
    "../aes/aeskey.c"
    "../aes/aes_modes.c"
    "../aes/aestab.c"
    "../aes/aes_amd64.asm"
    "aes_amd64.o"
    "cwc.c"
    "cwc.h"
    "mode_hdr.h"
)
source_group("" FILES ${no_group_source_files})

set(ALL_FILES
    ${no_group_source_files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "ThirdParty")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE aesmodes)

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
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
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
    "_CRT_SECURE_NO_WARNINGS;"
    "_CONSOLE;"
    "UNICODE;"
    "_UNICODE;"
    "ASM_AMD64_C;"
    "INTEL_AES_POSSIBLE"
)


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../aes")
################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )
endif()

