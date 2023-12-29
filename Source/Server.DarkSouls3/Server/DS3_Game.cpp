/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/DS3_Game.h"
#include "Server/Server.h"

#include "Server/GameService/DS3_PlayerState.h"

#include "Protobuf/DS3_Protobufs.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS3_GameIds.h"

#include "Server/GameService/GameManagers/Boot/DS3_BootManager.h"
#include "Server/GameService/GameManagers/Logging/DS3_LoggingManager.h"
#include "Server/GameService/GameManagers/PlayerData/DS3_PlayerDataManager.h"
#include "Server/GameService/GameManagers/BloodMessage/DS3_BloodMessageManager.h"
#include "Server/GameService/GameManagers/Bloodstain/DS3_BloodstainManager.h"
#include "Server/GameService/GameManagers/Ghosts/DS3_GhostManager.h"
#include "Server/GameService/GameManagers/Signs/DS3_SignManager.h"
#include "Server/GameService/GameManagers/Ranking/DS3_RankingManager.h"
#include "Server/GameService/GameManagers/QuickMatch/DS3_QuickMatchManager.h"
#include "Server/GameService/GameManagers/BreakIn/DS3_BreakInManager.h"
#include "Server/GameService/GameManagers/Visitor/DS3_VisitorManager.h"
#include "Server/GameService/GameManagers/Mark/DS3_MarkManager.h"
#include "Server/GameService/GameManagers/Misc/DS3_MiscManager.h"
#include "Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatManager.h"

#include <unordered_map>

bool DS3_Game::Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass) \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS3_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                 \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS3_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                            \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Push; /* Not using push */            \
        return true;                                                                \
    }    
#include "Server.DarkSouls3/Server/Streams/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS3_Game::ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType InType, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)                     \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ResponseProtobufClass>();                    \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ProtobufClass>();                            \
        }                                                                                               \
        return true;                                                                                    \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                                     \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ProtobufClass>();                            \
            return true;                                                                                \
        }                                                                                               \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                                /* Not supported on server, server only sends these */
#include "Server.DarkSouls3/Server/Streams/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS3_Game::ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Push)
    {                                                                                       
        return false;                                                                       
    }

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return false;                                                                       \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Not required, gets caught by Push test above */
#include "Server.DarkSouls3/Server/Streams/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

std::string DS3_Game::GetBossDiscordThumbnailUrl(uint32_t BaseBossId)
{
    static std::unordered_map<DS3_BossId, std::string> DiscordBossThumbnails = {
        { DS3_BossId::Spear_of_the_Church,              "https://i.imgur.com/mIOPfle.jpeg" },
        { DS3_BossId::Vordt_of_the_Boreal_Valley,       "https://i.imgur.com/Jzp2qJb.png" },
        { DS3_BossId::Oceiros_the_Consumed_King,        "https://i.imgur.com/bWnnWbq.png" },
        { DS3_BossId::Dancer_of_the_Boreal_Valley,      "https://i.imgur.com/EKcsNcx.png" },
        { DS3_BossId::Dragonslayer_Armour,              "https://i.imgur.com/EdFD3kY.png" },
        { DS3_BossId::Curse_rotted_Greatwood,           "https://i.imgur.com/BrPD6Ac.png" },
        { DS3_BossId::Ancient_Wyvern,                   "https://i.imgur.com/UDvq8KJ.png" },
        { DS3_BossId::King_of_the_Storm,                "https://i.imgur.com/8m67IlX.png" },
        { DS3_BossId::Nameless_King,                    "https://i.imgur.com/8m67IlX.png" },
        { DS3_BossId::Abyss_Watchers,                   "https://i.imgur.com/D7R7kEZ.png" },
        { DS3_BossId::Crystal_Sage,                     "https://i.imgur.com/vDundzi.png" },
        { DS3_BossId::Lorian_Elder_Prince,              "https://i.imgur.com/J2jflUK.png" },
        { DS3_BossId::Lothric_Younger_Prince,           "https://i.imgur.com/J2jflUK.png" },
        { DS3_BossId::Lorian_Elder_Prince_2,            "https://i.imgur.com/J2jflUK.png" },
        { DS3_BossId::Deacons_of_the_Deep,              "https://i.imgur.com/eH6g1TO.png" },
        { DS3_BossId::Aldrich_Devourer_Of_Gods,         "https://i.imgur.com/FczwbhQ.png" },
        { DS3_BossId::Pontiff_Sulyvahn,                 "https://i.imgur.com/VwOi0qc.png" },
        { DS3_BossId::High_Lord_Wolnir,                 "https://i.imgur.com/xdYcHi1.png" },
        { DS3_BossId::Old_Demon_King,                   "https://i.imgur.com/UGO8MEm.png" },
        { DS3_BossId::Yhorm_the_Giant,                  "https://i.imgur.com/KpJc8p2.png" },
        { DS3_BossId::Iudex_Gundyr,                     "https://i.imgur.com/eZ2KtGU.png" },
        { DS3_BossId::Champion_Gundyr,                  "https://i.imgur.com/ZX4KwhL.png" },
        { DS3_BossId::Soul_Of_Cinder,                   "https://i.imgur.com/KRKJAow.jpeg" },
        { DS3_BossId::Blackflame_Friede,                "https://i.imgur.com/Ja8rb4n.jpeg" },
        { DS3_BossId::Sister_Friede,                    "https://i.imgur.com/Ja8rb4n.jpeg" },
        { DS3_BossId::Father_Ariandel_And_Friede,       "https://i.imgur.com/Ja8rb4n.jpeg" },
        { DS3_BossId::Gravetender_Greatwolf,            "https://i.imgur.com/k5ngpvT.jpeg" },
        { DS3_BossId::Demon_From_Below,                 "https://i.imgur.com/D4NtdCf.jpeg" },
        { DS3_BossId::Demon_In_Pain,                    "https://i.imgur.com/D4NtdCf.jpeg" },
        { DS3_BossId::Halflight,                        "https://i.imgur.com/mIOPfle.jpeg" },
        { DS3_BossId::Darkeater_Midir,                  "https://i.imgur.com/uuMfxNQ.jpeg" },
        { DS3_BossId::Slave_Knight_Gael,                "https://i.imgur.com/5mmzjvy.jpeg" }
    };

    if (auto Iter = DiscordBossThumbnails.find((DS3_BossId)BaseBossId); Iter != DiscordBossThumbnails.end())
    {
        return Iter->second;
    }

    return "";
}

void DS3_Game::RegisterGameManagers(GameService& Service)
{
    Server* ServerInstance = Service.GetServer();

    Service.RegisterManager(std::make_shared<DS3_BootManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_LoggingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_PlayerDataManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_BloodMessageManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_BloodstainManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_SignManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_GhostManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_RankingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_QuickMatchManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_BreakInManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_VisitorManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_MarkManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS3_MiscManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS3_AntiCheatManager>(ServerInstance, &Service));
}

std::unique_ptr<PlayerState> DS3_Game::CreatePlayerState()
{
    return std::make_unique<DS3_PlayerState>();
}

std::string DS3_Game::GetAreaName(uint32_t AreaId)
{
    return GetEnumString<DS3_OnlineAreaId>((DS3_OnlineAreaId)AreaId);
}

void DS3_Game::GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats)
{
    std::shared_ptr<DS3_BloodMessageManager> BloodMessages = Service.GetManager<DS3_BloodMessageManager>();
    std::shared_ptr<DS3_BloodstainManager> Bloodstains = Service.GetManager<DS3_BloodstainManager>();
    std::shared_ptr<DS3_QuickMatchManager> QuickMatches = Service.GetManager<DS3_QuickMatchManager>();
    std::shared_ptr<DS3_SignManager> Signs = Service.GetManager<DS3_SignManager>();
    std::shared_ptr<DS3_GhostManager> Ghosts = Service.GetManager<DS3_GhostManager>();

    Stats["Live Blood Messages"] = std::to_string(BloodMessages->GetLiveCount());
    Stats["Live Blood Stains"] = std::to_string(Bloodstains->GetLiveCount());
    Stats["Live Undead Matches"] = std::to_string(QuickMatches->GetLiveCount());
    Stats["Live Summon Signs"] = std::to_string(Signs->GetLiveCount());
    Stats["Live Ghosts"] = std::to_string(Ghosts->GetLiveCount());
}

void DS3_Game::SendManagementMessage(Frpg2ReliableUdpMessageStream& Stream, const std::string& TextMessage)
{
    DS3_Frpg2RequestMessage::ManagementTextMessage Message;
    Message.set_push_message_id(DS3_Frpg2RequestMessage::PushID_ManagementTextMessage);
    Message.set_message(TextMessage);
    Message.set_unknown_4(0);
    Message.set_unknown_5(0);

    // Date makes no difference, just hard-code for now.
    DS3_Frpg2PlayerData::DateTime* DateTime = Message.mutable_timestamp();
    DateTime->set_year(2021);
    DateTime->set_month(1);
    DateTime->set_day(1);
    DateTime->set_hours(0);
    DateTime->set_minutes(0);
    DateTime->set_seconds(0);
    DateTime->set_tzdiff(0);

    Stream.Send(&Message);
}