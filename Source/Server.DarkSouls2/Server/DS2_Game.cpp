/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/DS2_Game.h"
#include "Protobuf/DS2_Protobufs.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/DS2_PlayerState.h"

#include "Server/GameService/GameManagers/Boot/DS2_BootManager.h"
#include "Server/GameService/GameManagers/PlayerData/DS2_PlayerDataManager.h"
#include "Server/GameService/GameManagers/Ghosts/DS2_GhostManager.h"
#include "Server/GameService/GameManagers/BloodMessage/DS2_BloodMessageManager.h"
#include "Server/GameService/GameManagers/Bloodstain/DS2_BloodstainManager.h"
#include "Server/GameService/GameManagers/Signs/DS2_SignManager.h"0
#include "Server/GameService/GameManagers/BreakIn/DS2_BreakInManager.h"
#include "Server/GameService/GameManagers/Logging/DS2_LoggingManager.h"
#include "Server/GameService/GameManagers/Misc/DS2_MiscManager.h"
#include "Server/GameService/GameManagers/Visitor/DS2_VisitorManager.h"
#include "Server/GameService/GameManagers/Ranking/DS2_RankingManager.h"
#include "Server/GameService/GameManagers/MirrorKnight/DS2_MirrorKnightManager.h"
#include "Server/GameService/GameManagers/QuickMatch/DS2_QuickMatchManager.h"

bool DS2_Game::Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass) \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS2_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                 \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS2_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                            \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Push; /* Not using push */            \
        return true;                                                                \
    }    
#include "Server.DarkSouls2/Server/Streams/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS2_Game::ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType InType, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)                     \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ResponseProtobufClass>();                    \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ProtobufClass>();                            \
        }                                                                                               \
        return true;                                                                                    \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                                     \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ProtobufClass>();                            \
            return true;                                                                                \
        }                                                                                               \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                                /* Not supported on server, server only sends these */
#include "Server.DarkSouls2/Server/Streams/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS2_Game::ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Push)
    {                                                                                       
        return false;                                                                       
    }

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return false;                                                                       \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Not required, gets caught by Push test above */
#include "Server.DarkSouls2/Server/Streams/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

std::string DS2_Game::GetBossDiscordThumbnailUrl(uint32_t BossId)
{
    // TODO
    
    return "";
}

void DS2_Game::RegisterGameManagers(GameService& Service)
{
    Server* ServerInstance = Service.GetServer();

    Service.RegisterManager(std::make_shared<DS2_BootManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_PlayerDataManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_GhostManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_BloodMessageManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_BloodstainManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_BreakInManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_LoggingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_MiscManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_VisitorManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_RankingManager>(ServerInstance));
    Service.RegisterManager(std::make_shared<DS2_MirrorKnightManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_SignManager>(ServerInstance, &Service));
    Service.RegisterManager(std::make_shared<DS2_QuickMatchManager>(ServerInstance, &Service));
    
}

std::unique_ptr<PlayerState> DS2_Game::CreatePlayerState()
{
    return std::make_unique<DS2_PlayerState>();
}

std::string DS2_Game::GetAreaName(uint32_t AreaId)
{
    return GetEnumString<DS2_OnlineAreaId>((DS2_OnlineAreaId)AreaId);
}

void DS2_Game::GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats)
{
    std::shared_ptr<DS2_BloodMessageManager> BloodMessages = Service.GetManager<DS2_BloodMessageManager>();
    std::shared_ptr<DS2_BloodstainManager> Bloodstains = Service.GetManager<DS2_BloodstainManager>();
    std::shared_ptr<DS2_QuickMatchManager> QuickMatches = Service.GetManager<DS2_QuickMatchManager>();
    std::shared_ptr<DS2_SignManager> Signs = Service.GetManager<DS2_SignManager>();
    std::shared_ptr<DS2_GhostManager> Ghosts = Service.GetManager<DS2_GhostManager>();

    Stats["Live Blood Messages"] = std::to_string(BloodMessages->GetLiveCount());
    Stats["Live Blood Stains"] = std::to_string(Bloodstains->GetLiveCount());
    Stats["Live Undead Matches"] = std::to_string(QuickMatches->GetLiveCount());
    Stats["Live Summon Signs"] = std::to_string(Signs->GetLiveCount());
    Stats["Live Ghosts"] = std::to_string(Ghosts->GetLiveCount());
}

void DS2_Game::SendManagementMessage(Frpg2ReliableUdpMessageStream& stream, const std::string& TextMessage) 
{
    DS2_Frpg2RequestMessage::ManagementTextMessage Message;
    Message.set_push_message_id(DS2_Frpg2RequestMessage::PushID_ManagementTextMessage);
    Message.set_message(TextMessage);
    Message.set_unknown_4(0);
    Message.set_unknown_5(0);

    // Date makes no difference, just hard-code for now.
    DS2_Frpg2PlayerData::DateTime* DateTime = Message.mutable_timestamp();
    DateTime->set_year(2021);
    DateTime->set_month(1);
    DateTime->set_day(1);
    DateTime->set_hours(0);
    DateTime->set_minutes(0);
    DateTime->set_seconds(0);
    DateTime->set_tzdiff(0);

    stream.Send(&Message);
}