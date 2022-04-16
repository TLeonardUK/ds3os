# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================

project(steam C CXX)

if (WIN32)
    set(STEAM_SHLIB "win64/steam_api64.dll")
    set(STEAM_LIB "win64/steam_api64.lib")
    set(STEAM_EAT_SHLIB "win64/sdkencryptedappticket64.dll")
    set(STEAM_EAT_LIB "win64/sdkencryptedappticket64.lib")
elseif(UNIX)
    set(STEAM_SHLIB "linux64/libsteam_api.so")
    set(STEAM_EAT_SHLIB "linux64/libsdkencryptedappticket.so")
endif()

add_library(${PROJECT_NAME} SHARED IMPORTED GLOBAL)

set_target_properties(${PROJECT_NAME} PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/redistributable_bin/${STEAM_SHLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/public"
)

if(STEAM_LIB)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/redistributable_bin/${STEAM_LIB}"
    )
endif()

add_library(${PROJECT_NAME}-encryptedappticket SHARED IMPORTED GLOBAL)

set_target_properties(${PROJECT_NAME}-encryptedappticket PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/public/steam/lib/${STEAM_EAT_SHLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/public"
)

if (STEAM_EAT_LIB)
    set_target_properties(${PROJECT_NAME}-encryptedappticket PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/public/steam/lib/${STEAM_EAT_LIB}"
    )
endif()
