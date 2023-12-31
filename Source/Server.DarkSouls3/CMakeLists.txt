# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(DarkSouls3 C CXX)

SET(SOURCES
    Protobuf/DS3_Protobufs.h
    Protobuf/Generated/DS3_FpdLogMessage.pb.cc
    Protobuf/Generated/DS3_Frpg2PlayerData.pb.cc
    Protobuf/Generated/DS3_Frpg2RequestMessage.pb.cc
    
    Server/GameService/DS3_PlayerState.h

    Server/GameService/GameManagers/BloodMessage/DS3_BloodMessageManager.cpp
    Server/GameService/GameManagers/BloodMessage/DS3_BloodMessageManager.h
    Server/GameService/GameManagers/Bloodstain/DS3_BloodstainManager.cpp
    Server/GameService/GameManagers/Bloodstain/DS3_BloodstainManager.h
    Server/GameService/GameManagers/Boot/DS3_BootManager.cpp
    Server/GameService/GameManagers/Boot/DS3_BootManager.h
    Server/GameService/GameManagers/BreakIn/DS3_BreakInManager.cpp
    Server/GameService/GameManagers/BreakIn/DS3_BreakInManager.h
    Server/GameService/GameManagers/Ghosts/DS3_GhostManager.cpp
    Server/GameService/GameManagers/Ghosts/DS3_GhostManager.h
    Server/GameService/GameManagers/Logging/DS3_LoggingManager.cpp
    Server/GameService/GameManagers/Logging/DS3_LoggingManager.h
    Server/GameService/GameManagers/Mark/DS3_MarkManager.cpp
    Server/GameService/GameManagers/Mark/DS3_MarkManager.h
    Server/GameService/GameManagers/Misc/DS3_MiscManager.cpp
    Server/GameService/GameManagers/Misc/DS3_MiscManager.h
    Server/GameService/GameManagers/PlayerData/DS3_PlayerDataManager.cpp
    Server/GameService/GameManagers/PlayerData/DS3_PlayerDataManager.h
    Server/GameService/GameManagers/QuickMatch/DS3_QuickMatchManager.cpp
    Server/GameService/GameManagers/QuickMatch/DS3_QuickMatchManager.h
    Server/GameService/GameManagers/Ranking/DS3_RankingManager.cpp
    Server/GameService/GameManagers/Ranking/DS3_RankingManager.h
    Server/GameService/GameManagers/Signs/DS3_SignManager.cpp
    Server/GameService/GameManagers/Signs/DS3_SignManager.h
    Server/GameService/GameManagers/Visitor/DS3_VisitorManager.cpp
    Server/GameService/GameManagers/Visitor/DS3_VisitorManager.h
    Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatManager.cpp
    Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatManager.h
    Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatTrigger.cpp
    Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatTrigger.h
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ClientFlagged.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ClientFlagged.h
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_Exploit.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_Exploit.h
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ImpossibleStats.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ImpossibleStats.h
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_InvalidName.cpp
    Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_InvalidName.h

    Server/GameService/Utils/DS3_GameIds.h
    Server/GameService/Utils/DS3_NRSSRSanitizer.h

    Server/GameService/Utils/Ids/BonfireId.inc
    Server/GameService/Utils/Ids/CharacterTypeId.inc
    Server/GameService/Utils/Ids/CovenantId.inc
    Server/GameService/Utils/Ids/GestureId.inc
    Server/GameService/Utils/Ids/InvasionTypeId.inc
    Server/GameService/Utils/Ids/ItemId.inc
    Server/GameService/Utils/Ids/OnlineAreaId.inc
    Server/GameService/Utils/Ids/TeamTypeId.inc
    Server/GameService/Utils/Ids/BossId.inc    
    Server/GameService/Utils/Ids/BloodMessageConjunction.inc
    Server/GameService/Utils/Ids/BloodMessageGesture.inc
    Server/GameService/Utils/Ids/BloodMessageTemplate.inc
    Server/GameService/Utils/Ids/BloodMessageWord.inc    

    Server/GameService/Utils/Data/DS3_BloodMessageData.h
    Server/GameService/Utils/Data/DS3_BloodMessageData.cpp
    
    Server/Streams/DS3_Frpg2ReliableUdpMessageTypes.inc
    Server/Streams/DS3_Frpg2ReliableUdpMessage.h
    
    Server/DS3_Game.h
    Server/DS3_Game.cpp
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
