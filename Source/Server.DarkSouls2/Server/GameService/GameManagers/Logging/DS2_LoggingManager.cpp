/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Logging/DS2_LoggingManager.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS2_GameIds.h"
#include "Server.DarkSouls2/Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"
#include "Server/Database/ServerDatabase.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS2_LoggingManager::DS2_LoggingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS2_LoggingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyBuyItem))
    {
        return Handle_RequestNotifyBuyItem(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyDeath))
    {
        return Handle_RequestNotifyDeath(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyDisconnectSession))
    {
        return Handle_RequestNotifyDisconnectSession(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyJoinGuestPlayer))
    {
        return Handle_RequestNotifyJoinGuestPlayer(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyJoinSession))
    {
        return Handle_RequestNotifyJoinSession(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyKillEnemy))
    {
        return Handle_RequestNotifyKillEnemy(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyKillPlayer))
    {
        return Handle_RequestNotifyKillPlayer(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyLeaveGuestPlayer))
    {
        return Handle_RequestNotifyLeaveGuestPlayer(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyLeaveSession))
    {
        return Handle_RequestNotifyLeaveSession(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyMirrorKnight))
    {
        return Handle_RequestNotifyMirrorKnight(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestNotifyOfflineDeathCount))
    {
        return Handle_RequestNotifyOfflineDeathCount(Client, Message);
    }
        
    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyBuyItem(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestNotifyBuyItem* Request = (DS2_Frpg2RequestMessage::RequestNotifyBuyItem*)Message.Protobuf.get();

    std::string StatisticKey = StringFormat("Item/TotalPurchased/Id=%u", Request->item_id());
    Database.AddGlobalStatistic(StatisticKey, Request->quantity());
    Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), Request->quantity());

    std::string TotalStatisticKey = StringFormat("Item/TotalPurchased");
    Database.AddGlobalStatistic(TotalStatisticKey, Request->quantity());
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), Request->quantity());

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyDeath(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestNotifyDeath* Request = (DS2_Frpg2RequestMessage::RequestNotifyDeath*)Message.Protobuf.get();

    std::string TotalStatisticKey = StringFormat("Player/TotalDeaths");
    Database.AddGlobalStatistic(TotalStatisticKey, 1);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), 1);

    // TODO: Implement the rest of things from ds3.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyDisconnectSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyDisconnectSession* Request = (DS2_Frpg2RequestMessage::RequestNotifyDisconnectSession*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyJoinGuestPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyJoinGuestPlayer* Request = (DS2_Frpg2RequestMessage::RequestNotifyJoinGuestPlayer*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyJoinSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyJoinSession* Request = (DS2_Frpg2RequestMessage::RequestNotifyJoinSession*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyKillEnemy(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestNotifyKillEnemy* Request = (DS2_Frpg2RequestMessage::RequestNotifyKillEnemy*)Message.Protobuf.get();

    int EnemyCount = 0;
    for (int i = 0; i < Request->enemy_count_size(); i++)
    {
        const DS2_Frpg2RequestMessage::RequestNotifyKillEnemy_Enemy_count& EnemyInfo = Request->enemy_count(i);

        std::string StatisticKey = StringFormat("Enemies/TotalKilled/Id=%u", EnemyInfo.enemy_id());
        Database.AddGlobalStatistic(StatisticKey, EnemyInfo.enemy_count());
        Database.AddPlayerStatistic(StatisticKey, Player.GetPlayerId(), EnemyInfo.enemy_count());

        EnemyCount += EnemyInfo.enemy_count();
    }

    std::string TotalStatisticKey = StringFormat("Enemies/TotalKilled");
    Database.AddGlobalStatistic(TotalStatisticKey, EnemyCount);
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), EnemyCount);

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyKillPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyKillPlayer* Request = (DS2_Frpg2RequestMessage::RequestNotifyKillPlayer*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyLeaveGuestPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyLeaveGuestPlayer* Request = (DS2_Frpg2RequestMessage::RequestNotifyLeaveGuestPlayer*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyLeaveSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestNotifyLeaveSession* Request = (DS2_Frpg2RequestMessage::RequestNotifyLeaveSession*)Message.Protobuf.get();

    std::string TypeStatisticKey = StringFormat("Player/TotalMultiplaySessions");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyMirrorKnight(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestNotifyMirrorKnight* Request = (DS2_Frpg2RequestMessage::RequestNotifyMirrorKnight*)Message.Protobuf.get();

    // Note: I don't think we really care about this log. We get most of this during the summon flow.

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_LoggingManager::Handle_RequestNotifyOfflineDeathCount(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestNotifyOfflineDeathCount* Request = (DS2_Frpg2RequestMessage::RequestNotifyOfflineDeathCount*)Message.Protobuf.get();

    std::string TotalStatisticKey = StringFormat("Player/TotalDeaths");
    Database.AddGlobalStatistic(TotalStatisticKey, Request->count());
    Database.AddPlayerStatistic(TotalStatisticKey, Player.GetPlayerId(), Request->count());

    DS2_Frpg2RequestMessage::EmptyResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send EmptyResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_LoggingManager::GetName()
{
    return "Logging";
}
