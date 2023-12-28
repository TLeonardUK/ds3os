# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(DarkSouls2 C CXX)

SET(SOURCES
    Protobuf/DarkSouls2/DS2_Frpg2PlayerData.pb.cc
    Protobuf/DarkSouls2/DS2_Frpg2RequestMessage.pb.cc

    Server/Streams/DarkSouls2/DS2_Frpg2ReliableUdpMessageTypes.inc
)
 
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Server"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_libraries(
    ${PROJECT_NAME}
    Shared
    civetweb
    libprotobuf-lite
    sqlite
    steam
)

util_setup_folder_structure(${PROJECT_NAME} SOURCES "Games")
