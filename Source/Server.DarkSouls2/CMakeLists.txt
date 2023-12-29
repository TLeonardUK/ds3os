# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(DarkSouls2 C CXX)

SET(SOURCES
    Protobuf/DS2_Protobufs.h
    Protobuf/Generated/DS2_Frpg2PlayerData.pb.cc
    Protobuf/Generated/DS2_Frpg2RequestMessage.pb.cc
    
    Server/GameService/DS2_PlayerState.h

    Server/Streams/DS2_Frpg2ReliableUdpMessageTypes.inc
    Server/Streams/DS2_Frpg2ReliableUdpMessage.h
    
    Server/DS2_Game.h
    Server/DS2_Game.cpp
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
