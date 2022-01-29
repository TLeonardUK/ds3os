

include(CMake/Utils.cmake)

add_library(steam_api SHARED IMPORTED)
set(STEAMWORKS_SDK "${PROJECT_SOURCE_DIR}/third-party/steamworks")

if (WIN32)
    set(STEAM_SHLIB "win64/steam_api64.dll")
    set(STEAM_LIB "win64/steam_api64.lib")
elseif(APPLE)
elseif(UNIX)
    set(STEAM_SHLIB "linux64/libsteam_api.so")
endif()

set_target_properties(steam_api PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Source/ThirdParty/steam/redistributable_bin/${STEAM_SHLIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/Source/ThirdParty/steam/public")

if(STEAM_LIB)
    set_target_properties(steam_api PROPERTIES
            IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Source/ThirdParty/steam/redistributable_bin/${STEAM_LIB}")
endif()

add_subdirectory(Server)
add_subdirectory(ThirdParty/aes/lib_generic_c)
add_subdirectory(ThirdParty/aes_modes)


