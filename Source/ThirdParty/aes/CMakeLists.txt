# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(lib_generic_c C CXX)

SET(SOURCES
    aes.h
    aes_ni.h
    aes_via_ace.h
    aesopt.h
    aestab.h
    brg_endian.h
    brg_types.h
    aes_modes.c
    aes_ni.c
    aescrypt.c
    aeskey.c
    aestab.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
