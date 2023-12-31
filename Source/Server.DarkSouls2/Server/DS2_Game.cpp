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
}

std::unique_ptr<PlayerState> DS2_Game::CreatePlayerState()
{
    return std::make_unique<DS2_PlayerState>();
}

std::string DS2_Game::GetAreaName(uint32_t AreaId)
{
    // TODO

    return "";
}

void DS2_Game::GetStatistics(GameService& Service, std::unordered_map<std::string, std::string>& Stats)
{
    // TODO
}

void DS2_Game::SendManagementMessage(Frpg2ReliableUdpMessageStream& stream, const std::string& TextMessage) 
{
    // TODO
}