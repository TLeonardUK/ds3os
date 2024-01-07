# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(Server C CXX)

SET(SOURCES
    Entry.cpp

    Client/Client.cpp
    Client/Client.h

    Config/BuildConfig.h
    Config/RuntimeConfig.h
    Config/RuntimeConfig.cpp

    Protobuf/Generated/Shared_FpdLogMessage.pb.cc
    Protobuf/Generated/Shared_Frpg2PlayerData.pb.cc
    Protobuf/Generated/Shared_Frpg2RequestMessage.pb.cc
    
    Protobuf/SharedProtobufs.h
        
    Server/Game.cpp
    Server/Game.h
    Server/ServerManager.cpp
    Server/ServerManager.h
    Server/Server.cpp
    Server/Server.h
    Server/Service.h
    Server/AuthService/AuthClient.cpp
    Server/AuthService/AuthClient.h
    Server/AuthService/AuthService.cpp
    Server/AuthService/AuthService.h
    Server/Database/DatabaseTypes.h
    Server/Database/ServerDatabase.cpp
    Server/Database/ServerDatabase.h
    Server/GameService/GameClient.cpp
    Server/GameService/GameClient.h
    Server/GameService/GameManager.h
    Server/GameService/GameService.cpp
    Server/GameService/GameService.h
    Server/GameService/PlayerState.h
    Server/GameService/Utils/OnlineAreaPool.h

    Server/LoginService/LoginClient.cpp
    Server/LoginService/LoginClient.h
    Server/LoginService/LoginService.cpp
    Server/LoginService/LoginService.h
    Server/Streams/Frpg2Message.h
    Server/Streams/Frpg2MessageStream.cpp
    Server/Streams/Frpg2MessageStream.h
    Server/Streams/Frpg2Packet.h
    Server/Streams/Frpg2PacketStream.cpp
    Server/Streams/Frpg2PacketStream.h
    Server/Streams/Frpg2ReliableUdpFragment.h
    Server/Streams/Frpg2ReliableUdpFragmentStream.cpp
    Server/Streams/Frpg2ReliableUdpFragmentStream.h
    Server/Streams/Frpg2ReliableUdpMessage.cpp
    Server/Streams/Frpg2ReliableUdpMessage.h
    Server/Streams/Frpg2ReliableUdpMessageStream.cpp
    Server/Streams/Frpg2ReliableUdpMessageStream.h
    Server/Streams/Frpg2ReliableUdpPacket.cpp
    Server/Streams/Frpg2ReliableUdpPacket.h
    Server/Streams/Frpg2ReliableUdpPacketStream.cpp
    Server/Streams/Frpg2ReliableUdpPacketStream.h
    
    Server/Streams/Frpg2UdpPacket.h
    Server/Streams/Frpg2UdpPacketStream.cpp
    Server/Streams/Frpg2UdpPacketStream.h
    Server/Streams/README
    Server/WebUIService/WebUIService.cpp
    Server/WebUIService/WebUIService.h
    Server/WebUIService/Handlers/AuthHandler.cpp
    Server/WebUIService/Handlers/AuthHandler.h
    Server/WebUIService/Handlers/DebugStatisticsHandler.cpp
    Server/WebUIService/Handlers/DebugStatisticsHandler.h
    Server/WebUIService/Handlers/MessageHandler.cpp
    Server/WebUIService/Handlers/MessageHandler.h
    Server/WebUIService/Handlers/BansHandler.cpp
    Server/WebUIService/Handlers/BansHandler.h
    Server/WebUIService/Handlers/PlayersHandler.cpp
    Server/WebUIService/Handlers/PlayersHandler.h
    Server/WebUIService/Handlers/SettingsHandler.cpp
    Server/WebUIService/Handlers/SettingsHandler.h
    Server/WebUIService/Handlers/StatisticsHandler.cpp
    Server/WebUIService/Handlers/StatisticsHandler.h
    Server/WebUIService/Handlers/ShardingHandler.cpp
    Server/WebUIService/Handlers/ShardingHandler.h
    Server/WebUIService/Handlers/WebUIHandler.cpp
    Server/WebUIService/Handlers/WebUIHandler.h
)
 
add_executable(${PROJECT_NAME} ${SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_libraries(
    ${PROJECT_NAME}
    DarkSouls3
    DarkSouls2
    Shared
    civetweb
    libprotobuf-lite
    sqlite
    steam
)

# Copy some important files over to our output directory.
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config_upper)
    
    # Write steam appid file, required for dedicated servers.
    # Server writes this out automatically now.
    #file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}/steam_appid.txt" "374320")

    # Copy WebUI resources to output folder
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../WebUI" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}")
endforeach()

util_setup_folder_structure(${PROJECT_NAME} SOURCES "DS3OS")
