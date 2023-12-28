# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(DarkSouls3 C CXX)

SET(SOURCES
    Protobuf/DarkSouls3/DS3_FpdLogMessage.pb.cc
    Protobuf/DarkSouls3/DS3_Frpg2PlayerData.pb.cc
    Protobuf/DarkSouls3/DS3_Frpg2RequestMessage.pb.cc
    
    Server/GameService/DarkSouls3/GameManagers/BloodMessage/BloodMessageManager.cpp
    Server/GameService/DarkSouls3/GameManagers/BloodMessage/BloodMessageManager.h
    Server/GameService/DarkSouls3/GameManagers/Bloodstain/BloodstainManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Bloodstain/BloodstainManager.h
    Server/GameService/DarkSouls3/GameManagers/Boot/BootManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Boot/BootManager.h
    Server/GameService/DarkSouls3/GameManagers/BreakIn/BreakInManager.cpp
    Server/GameService/DarkSouls3/GameManagers/BreakIn/BreakInManager.h
    Server/GameService/DarkSouls3/GameManagers/Ghosts/GhostManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Ghosts/GhostManager.h
    Server/GameService/DarkSouls3/GameManagers/Logging/LoggingManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Logging/LoggingManager.h
    Server/GameService/DarkSouls3/GameManagers/Mark/MarkManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Mark/MarkManager.h
    Server/GameService/DarkSouls3/GameManagers/Misc/MiscManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Misc/MiscManager.h
    Server/GameService/DarkSouls3/GameManagers/PlayerData/PlayerDataManager.cpp
    Server/GameService/DarkSouls3/GameManagers/PlayerData/PlayerDataManager.h
    Server/GameService/DarkSouls3/GameManagers/QuickMatch/QuickMatchManager.cpp
    Server/GameService/DarkSouls3/GameManagers/QuickMatch/QuickMatchManager.h
    Server/GameService/DarkSouls3/GameManagers/Ranking/RankingManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Ranking/RankingManager.h
    Server/GameService/DarkSouls3/GameManagers/Signs/SignManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Signs/SignManager.h
    Server/GameService/DarkSouls3/GameManagers/Visitor/VisitorManager.cpp
    Server/GameService/DarkSouls3/GameManagers/Visitor/VisitorManager.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatManager.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatManager.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatTrigger.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatTrigger.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ClientFlagged.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ClientFlagged.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_Exploit.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_Exploit.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ImpossibleStats.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ImpossibleStats.h
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_InvalidName.cpp
    Server/GameService/DarkSouls3/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_InvalidName.h
    Server/GameService/DarkSouls3/Utils/GameIds.h
    Server/GameService/DarkSouls3/Utils/NRSSRSanitizer.h
    Server/GameService/DarkSouls3/Utils/OnlineAreaPool.h
    Server/GameService/DarkSouls3/Utils/Ids/BonfireId.inc
    Server/GameService/DarkSouls3/Utils/Ids/CharacterTypeId.inc
    Server/GameService/DarkSouls3/Utils/Ids/ConvenantId.h
    Server/GameService/DarkSouls3/Utils/Ids/CovenantId.h
    Server/GameService/DarkSouls3/Utils/Ids/CovenantId.inc
    Server/GameService/DarkSouls3/Utils/Ids/GestureId.inc
    Server/GameService/DarkSouls3/Utils/Ids/InvasionTypeId.inc
    Server/GameService/DarkSouls3/Utils/Ids/ItemId.h
    Server/GameService/DarkSouls3/Utils/Ids/ItemId.inc
    Server/GameService/DarkSouls3/Utils/Ids/OnlineAreaId.inc
    Server/GameService/DarkSouls3/Utils/Ids/TeamTypeId.inc
    Server/GameService/DarkSouls3/Utils/Ids/BossId.inc    
    Server/GameService/DarkSouls3/Utils/Ids/BloodMessageConjunction.inc
    Server/GameService/DarkSouls3/Utils/Ids/BloodMessageGesture.inc
    Server/GameService/DarkSouls3/Utils/Ids/BloodMessageTemplate.inc
    Server/GameService/DarkSouls3/Utils/Ids/BloodMessageWord.inc    
    Server/GameService/DarkSouls3/Utils/Data/BloodMessageData.h
    Server/GameService/DarkSouls3/Utils/Data/BloodMessageData.cpp
    
    Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc
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
