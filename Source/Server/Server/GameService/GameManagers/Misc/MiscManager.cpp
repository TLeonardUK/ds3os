/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Misc/MiscManager.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

MiscManager::MiscManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

MessageHandleResult MiscManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyRingBell)
    {
        return Handle_RequestNotifyRingBell(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendMessageToPlayers)
    {
        return Handle_RequestSendMessageToPlayers(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestMeasureUploadBandwidth)
    {
        return Handle_RequestMeasureUploadBandwidth(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestMeasureDownloadBandwidth)
    {
        return Handle_RequestMeasureDownloadBandwidth(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetOnlineShopItemList)
    {
        return Handle_RequestGetOnlineShopItemList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestBenchmarkThroughput)
    {
        return Handle_RequestBenchmarkThroughput(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult MiscManager::Handle_RequestNotifyRingBell(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyRingBell* Request = (Frpg2RequestMessage::RequestNotifyRingBell*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestNotifyRingBellResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestNotifyRingBellResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestSendMessageToPlayers(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestSendMessageToPlayers* Request = (Frpg2RequestMessage::RequestSendMessageToPlayers*)Message.Protobuf.get();

    // Wow this whole function seems unsafe, no idea why from software allows this to be a thing.

    std::vector<uint8_t> MessageData;
    MessageData.assign(Request->message().data(), Request->message().data() + Request->message().size());

    for (int i = 0; i < Request->player_ids_size(); i++)
    {
        uint32_t PlayerId = Request->player_ids(i);

        std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(PlayerId);
        if (!TargetClient)
        {
            Warning("[%s] Client attempted to send message to other client %i, but client doesn't exist.", Client->GetName().c_str(), PlayerId);
        }
        else
        {
            if (!TargetClient->MessageStream->SendRawProtobuf(MessageData))
            {
                Warning("[%s] Failed to send raw protobuf from RequestSendMessageToPlayers to %s.", Client->GetName().c_str(), TargetClient->GetName().c_str());
            }
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    Frpg2RequestMessage::RequestSendMessageToPlayersResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestSendMessageToPlayersResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestMeasureUploadBandwidth(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestMeasureUploadBandwidth* Request = (Frpg2RequestMessage::RequestMeasureUploadBandwidth*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestMeasureUploadBandwidthResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestMeasureUploadBandwidthResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestMeasureDownloadBandwidth(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestMeasureDownloadBandwidth* Request = (Frpg2RequestMessage::RequestMeasureDownloadBandwidth*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestMeasureDownloadBandwidthResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestMeasureDownloadBandwidthResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestGetOnlineShopItemList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetOnlineShopItemList* Request = (Frpg2RequestMessage::RequestGetOnlineShopItemList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetOnlineShopItemListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetOnlineShopItemListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MiscManager::Handle_RequestBenchmarkThroughput(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestBenchmarkThroughput* Request = (Frpg2RequestMessage::RequestBenchmarkThroughput*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestBenchmarkThroughputResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestBenchmarkThroughputResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string MiscManager::GetName()
{
    return "Misc";
}
