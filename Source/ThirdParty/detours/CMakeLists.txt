# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(detours C CXX)

SET(SOURCES
    src/creatwth.cpp
    src/detours.cpp
    src/detours.h
    src/detver.h
    src/disasm.cpp
    src/disolarm.cpp
    src/disolarm64.cpp
    src/disolia64.cpp
    src/disolx64.cpp
    src/disolx86.cpp
    src/image.cpp
    src/modules.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

util_setup_folder_structure(${PROJECT_NAME} SOURCES "ThirdParty")
