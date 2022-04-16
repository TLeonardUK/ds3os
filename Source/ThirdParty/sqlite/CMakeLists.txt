# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(sqlite C CXX)

SET(SOURCES
    shell.c
    sqlite3.c
    sqlite3.h
    sqlite3ext.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
