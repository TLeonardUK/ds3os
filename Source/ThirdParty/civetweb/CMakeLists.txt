# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(civetweb C CXX)

SET(SOURCES
    "include/CivetServer.h"
    "include/civetweb.h"

    "src/CivetServer.cpp"
    "src/civetweb.c"
    "src/handle_form.inl"
    "src/md5.inl"
    "src/response.inl"
    "src/sha1.inl"
    "src/timer.inl"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_SSL=1)

target_include_directories(${PROJECT_NAME} PRIVATE include)

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
