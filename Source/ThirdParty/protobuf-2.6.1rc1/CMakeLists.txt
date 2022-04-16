# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(libprotobuf-lite C CXX)

SET(SOURCES
    vsprojects/config.h

    src/google/protobuf/io/coded_stream.h
    src/google/protobuf/io/coded_stream_inl.h
    src/google/protobuf/io/zero_copy_stream.h
    src/google/protobuf/io/zero_copy_stream_impl_lite.h

    src/google/protobuf/stubs/atomicops.h
    src/google/protobuf/stubs/atomicops_internals_x86_msvc.h
    src/google/protobuf/stubs/common.h
    src/google/protobuf/stubs/hash.h
    src/google/protobuf/stubs/map_util.h
    src/google/protobuf/stubs/once.h
    src/google/protobuf/stubs/platform_macros.h
    src/google/protobuf/stubs/stl_util.h
    src/google/protobuf/stubs/stringprintf.h
    src/google/protobuf/stubs/template_util.h
    src/google/protobuf/stubs/type_traits.h
    
    src/google/protobuf/extension_set.h
    src/google/protobuf/generated_message_util.h
    src/google/protobuf/message_lite.h
    src/google/protobuf/repeated_field.h
    src/google/protobuf/wire_format_lite.h
    src/google/protobuf/wire_format_lite_inl.h

    src/google/protobuf/io/coded_stream.cc
    src/google/protobuf/io/zero_copy_stream.cc
    src/google/protobuf/io/zero_copy_stream_impl_lite.cc

    src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
    src/google/protobuf/stubs/common.cc
    src/google/protobuf/stubs/once.cc
    src/google/protobuf/stubs/stringprintf.cc

    src/google/protobuf/repeated_field.cc
    src/google/protobuf/extension_set.cc
    src/google/protobuf/generated_message_util.cc
    src/google/protobuf/message_lite.cc
    src/google/protobuf/wire_format_lite.cc
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC src vsprojects)

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS -D_USRDLL -DLIBPROTOBUF_EXPORTS)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305)
endif()

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
