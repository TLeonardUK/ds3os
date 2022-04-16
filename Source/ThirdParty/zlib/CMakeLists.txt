# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(zlib C CXX)

SET(SOURCES
    adler32.c
    compress.c
    crc32.c
    crc32.h
    deflate.c
    deflate.h
    gzclose.c
    gzguts.h
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inffast.h
    inffixed.h
    inflate.c
    inflate.h
    inftrees.c
    inftrees.h
    trees.c
    trees.h
    uncompr.c
    zconf.h
    zlib.h
    zutil.c
    zutil.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4267)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_NONSTDC_NO_DEPRECATE=1 -D_CRT_SECURE_NO_WARNINGS=1)

target_include_directories(${PROJECT_NAME} PUBLIC ./)

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
