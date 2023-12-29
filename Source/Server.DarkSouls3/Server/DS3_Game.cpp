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

#include "Server/GameService/DarkSouls3/DS3_PlayerState.h"

#include "Protobuf/DS3_Protobufs.h"
#include "Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/DarkSouls3/Utils/GameIds.h"

#include "Server/GameService/DarkSouls3/GameManagers/Boot/BootManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Logging/LoggingManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/PlayerData/PlayerDataManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/BloodMessage/BloodMessageManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Bloodstain/BloodstainManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Ghosts/GhostManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Signs/SignManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Ranking/RankingManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/QuickMatch/QuickMatchManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/BreakIn/BreakInManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Visitor/VisitorManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Mark/MarkManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Misc/MiscManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatManager.h"

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
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
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
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
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
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

std::string DS3_Game::GetBossDiscordThumbnailUrl(uint32_t BaseBossId)
{
    static std::unordered_map<BossId, std::string> DiscordBossThumbnails = {
        { BossId::Spear_of_the_Church,              "https://i.imgur.com/mIOPfle.jpeg" },
        { BossId::Vordt_of_the_Boreal_Valley,       "https://i.imgur.com/Jzp2qJb.png" },
        { BossId::Oceiros_the_Consumed_King,        "https://i.imgur.com/bWnnWbq.png" },
        { BossId::Dancer_of_the_Boreal_Valley,      "https://i.imgur.com/EKcsNcx.png" },
        { BossId::Dragonslayer_Armour,              "https://i.imgur.com/EdFD3kY.png" },
        { BossId::Curse_rotted_Greatwood,           "https://i.imgur.com/BrPD6Ac.png" },
        { BossId::Ancient_Wyvern,                   "https://i.imgur.com/UDvq8KJ.png" },
        { BossId::King_of_the_Storm,                "https://i.imgur.com/8m67IlX.png" },
        { BossId::Nameless_King,                    "https://i.imgur.com/8m67IlX.png" },
        { BossId::Abyss_Watchers,                   "https://i.imgur.com/D7R7kEZ.png" },
        { BossId::Crystal_Sage,                     "https://i.imgur.com/vDundzi.png" },
        { BossId::Lorian_Elder_Prince,              "https://i.imgur.com/J2jflUK.png" },
        { BossId::Lothric_Younger_Prince,           "https://i.imgur.com/J2jflUK.png" },
        { BossId::Lorian_Elder_Prince_2,            "https://i.imgur.com/J2jflUK.png" },
        { BossId::Deacons_of_the_Deep,              "https://i.imgur.com/eH6g1TO.png" },
        { BossId::Aldrich_Devourer_Of_Gods,         "https://i.imgur.com/FczwbhQ.png" },
        { BossId::Pontiff_Sulyvahn,                 "https://i.imgur.com/VwOi0qc.png" },
        { BossId::High_Lord_Wolnir,                 "https://i.imgur.com/xdYcHi1.png" },
        { BossId::Old_Demon_King,                   "https://i.imgur.com/UGO8MEm.png" },
        { BossId::Yhorm_the_Giant,                  "https://i.imgur.com/KpJc8p2.png" },
        { BossId::Iudex_Gundyr,                     "https://i.imgur.com/eZ2KtGU.png" },
        { BossId::Champion_Gundyr,                  "https://i.imgur.com/ZX4KwhL.png" },
        { BossId::Soul_Of_Cinder,                   "https://i.imgur.com/KRKJAow.jpeg" },
        { BossId::Blackflame_Friede,                "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Sister_Friede,                    "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Father_Ariandel_And_Friede,       "https://i.imgur.com/Ja8rb4n.jpeg" },
        { BossId::Gravetender_Greatwolf,            "https://i.imgur.com/k5ngpvT.jpeg" },
        { BossId::Demon_From_Below,                 "https://i.imgur.com/D4NtdCf.jpeg" },
        { BossId::Demon_In_Pain,                    "https://i.imgur.com/D4NtdCf.jpeg" },
        { BossId::Halflight,                        "https://i.imgur.com/mIOPfle.jpeg" },
        { BossId::Darkeater_Midir,                  "https://i.imgur.com/uuMfxNQ.jpeg" },
        { BossId::Slave_Knight_Gael,                "https://i.imgur.com/5mmzjvy.jpeg" }
    };

    if (auto Iter = DiscordBossThumbnails.find((BossId)BaseBossId); Iter != DiscordBossThumbnails.end())
    {
        return Iter->second;
    }

    return "";
}

void DS3_Game::RegisterGameManagers(GameService& Service)
{
    Server* ServerInstance = Service.GetServer();

    Service.RegisterManager(std::make_shared<BootManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<LoggingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<PlayerDataManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<BloodMessageManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<BloodstainManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<SignManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<GhostManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<RankingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<QuickMatchManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<BreakInManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<VisitorManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<MarkManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<MiscManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<AntiCheatManager>(ServerInstance, &Service));
}

std::unique_ptr<PlayerState> DS3_Game::CreatePlayerState()
{
    return std::make_unique<DS3_PlayerState>();
}

std::string DS3_Game::GetAreaName(uint32_t AreaId)
{
    return GetEnumString<OnlineAreaId>((OnlineAreaId)AreaId);
}

void DS3_Game::GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats)
{
    std::shared_ptr<BloodMessageManager> BloodMessages = Service.GetManager<BloodMessageManager>();
    std::shared_ptr<BloodstainManager> Bloodstains = Service.GetManager<BloodstainManager>();
    std::shared_ptr<QuickMatchManager> QuickMatches = Service.GetManager<QuickMatchManager>();
    std::shared_ptr<SignManager> Signs = Service.GetManager<SignManager>();
    std::shared_ptr<GhostManager> Ghosts = Service.GetManager<GhostManager>();

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