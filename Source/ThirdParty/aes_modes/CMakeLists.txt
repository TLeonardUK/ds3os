# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(aes_modes C CXX)

SET(SOURCES
    cwc.c
    cwc.h
    mode_hdr.h
)
 
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ../aes ./)

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
