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

    Core/Crypto/Cipher.h
    Core/Crypto/CWCCipher.cpp
    Core/Crypto/CWCCipher.h
    Core/Crypto/CWCClientUDPCipher.cpp
    Core/Crypto/CWCClientUDPCipher.h
    Core/Crypto/CWCServerUDPCipher.cpp
    Core/Crypto/CWCServerUDPCipher.h
    Core/Crypto/RSACipher.cpp
    Core/Crypto/RSACipher.h
    Core/Crypto/RSAKeyPair.cpp
    Core/Crypto/RSAKeyPair.h
    Core/Network/NetConnection.h
    Core/Network/NetConnectionTCP.cpp
    Core/Network/NetConnectionTCP.h
    Core/Network/NetConnectionUDP.cpp
    Core/Network/NetConnectionUDP.h
    Core/Network/NetHttpRequest.cpp
    Core/Network/NetHttpRequest.h
    Core/Network/NetIPAddress.cpp
    Core/Network/NetIPAddress.h
    Core/Network/NetUtils.cpp
    Core/Network/NetUtils.h
    Core/Utils/Compression.cpp
    Core/Utils/Compression.h
    Core/Utils/DebugCounter.cpp
    Core/Utils/DebugCounter.h
    Core/Utils/DebugObjects.cpp
    Core/Utils/DebugObjects.h
    Core/Utils/DebugObjects.inc
    Core/Utils/DebugTimer.cpp
    Core/Utils/DebugTimer.h
    Core/Utils/DiffTracker.cpp
    Core/Utils/DiffTracker.h
    Core/Utils/Endian.h
    Core/Utils/Enum.h
    Core/Utils/EnumDef.inc
    Core/Utils/Event.h
    Core/Utils/File.cpp
    Core/Utils/File.h
    Core/Utils/Logging.cpp
    Core/Utils/Logging.h
    Core/Utils/Random.cpp
    Core/Utils/Random.h
    Core/Utils/Strings.cpp
    Core/Utils/Strings.h
    Core/Utils/Strings.h
    Core/Utils/Strings.h

    Platform/Platform.h

    Protobuf/FpdLogMessage.cc
    Protobuf/Frpg2PlayerData.cc
    Protobuf/Frpg2RequestMessage.cc
    Protobuf/Protobufs.h
    
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
    Server/GameService/GameManagers/BloodMessage/BloodMessageManager.cpp
    Server/GameService/GameManagers/BloodMessage/BloodMessageManager.h
    Server/GameService/GameManagers/Bloodstain/BloodstainManager.cpp
    Server/GameService/GameManagers/Bloodstain/BloodstainManager.h
    Server/GameService/GameManagers/Boot/BootManager.cpp
    Server/GameService/GameManagers/Boot/BootManager.h
    Server/GameService/GameManagers/BreakIn/BreakInManager.cpp
    Server/GameService/GameManagers/BreakIn/BreakInManager.h
    Server/GameService/GameManagers/Ghosts/GhostManager.cpp
    Server/GameService/GameManagers/Ghosts/GhostManager.h
    Server/GameService/GameManagers/Logging/LoggingManager.cpp
    Server/GameService/GameManagers/Logging/LoggingManager.h
    Server/GameService/GameManagers/Mark/MarkManager.cpp
    Server/GameService/GameManagers/Mark/MarkManager.h
    Server/GameService/GameManagers/Misc/MiscManager.cpp
    Server/GameService/GameManagers/Misc/MiscManager.h
    Server/GameService/GameManagers/PlayerData/PlayerDataManager.cpp
    Server/GameService/GameManagers/PlayerData/PlayerDataManager.h
    Server/GameService/GameManagers/QuickMatch/QuickMatchManager.cpp
    Server/GameService/GameManagers/QuickMatch/QuickMatchManager.h
    Server/GameService/GameManagers/Ranking/RankingManager.cpp
    Server/GameService/GameManagers/Ranking/RankingManager.h
    Server/GameService/GameManagers/Signs/SignManager.cpp
    Server/GameService/GameManagers/Signs/SignManager.h
    Server/GameService/GameManagers/Visitor/VisitorManager.cpp
    Server/GameService/GameManagers/Visitor/VisitorManager.h
    Server/GameService/GameManagers/AntiCheat/AntiCheatManager.cpp
    Server/GameService/GameManagers/AntiCheat/AntiCheatManager.h
    Server/GameService/GameManagers/AntiCheat/AntiCheatTrigger.cpp
    Server/GameService/GameManagers/AntiCheat/AntiCheatTrigger.h
    Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ClientFlag.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ClientFlag.h
    Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ImpossibleStats.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ImpossibleStats.h
    Server/GameService/Utils/GameIds.h
    Server/GameService/Utils/NRSSRSanitizer.h
    Server/GameService/Utils/OnlineAreaPool.h
    Server/GameService/Utils/Ids/BonfireId.inc
    Server/GameService/Utils/Ids/CharacterTypeId.inc
    Server/GameService/Utils/Ids/ConvenantId.h
    Server/GameService/Utils/Ids/CovenantId.h
    Server/GameService/Utils/Ids/CovenantId.inc
    Server/GameService/Utils/Ids/GestureId.inc
    Server/GameService/Utils/Ids/InvasionTypeId.inc
    Server/GameService/Utils/Ids/ItemId.h
    Server/GameService/Utils/Ids/ItemId.inc
    Server/GameService/Utils/Ids/OnlineAreaId.inc
    Server/GameService/Utils/Ids/TeamTypeId.inc
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
    Server/Streams/Frpg2ReliableUdpMessageTypes.inc
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
    Server/WebUIService/Handlers/PlayersHandler.cpp
    Server/WebUIService/Handlers/PlayersHandler.h
    Server/WebUIService/Handlers/SettingsHandler.cpp
    Server/WebUIService/Handlers/SettingsHandler.h
    Server/WebUIService/Handlers/StatisticsHandler.cpp
    Server/WebUIService/Handlers/StatisticsHandler.h
    Server/WebUIService/Handlers/WebUIHandler.cpp
    Server/WebUIService/Handlers/WebUIHandler.h
)
 
if (WIN32)
    SET(PLATFORM_SOURCES
        Platform/Win32/Win32Platform.cpp
    )
elseif (UNIX)
    SET(PLATFORM_SOURCES
        Platform/Linux/LinuxPlatform.cpp
    )
endif()
 
add_executable(${PROJECT_NAME} ${SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../")

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_libraries(
    ${PROJECT_NAME}
    aes_modes
    civetweb
    lib_generic_c
    libprotobuf-lite
    sqlite
    zlib
    steam
    #steam-encryptedappticket
    libcurl
    crypto
    ssl
)

# Copy some important files over to our output directory.
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config_upper)
    
    # Write steam appid file, required for dedicated servers.
    file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}/steam_appid.txt" "374320")

    # Copy WebUI resources to output folder
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../WebUI" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}")
endforeach()

util_setup_folder_structure(${PROJECT_NAME} SOURCES "DS3OS")
