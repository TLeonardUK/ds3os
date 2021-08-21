/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Signs/SignManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

SignManager::SignManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().SummonSignMaxEntriesPerArea);
}

void SignManager::OnLostPlayer(GameClient* Client)
{
    // Remove all the players signs from the cache.
    for (std::shared_ptr<SummonSign> Sign : Client->ActiveSummonSigns)
    {
        LiveCache.Remove(Sign->OnlineAreaId, Sign->SignId);
    }
    Client->ActiveSummonSigns.clear();
}

void SignManager::Poll()
{
    /*static float Timer = GetSeconds();
    if (GetSeconds() - Timer > 10.0f)
    {
        if (std::shared_ptr<GameClient> Client = GameServiceInstance->FindClientByPlayerId(2))
        {
            if (Client->ActiveSummonSigns.size() > 0)
            {
                std::shared_ptr<SummonSign> Sign = Client->ActiveSummonSigns[Client->ActiveSummonSigns.size() - 1];

                Frpg2RequestMessage::PushRequestSummonSign PushMessage;
                PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestSummonSign);
                PushMessage.mutable_message()->set_player_id(1);
                PushMessage.mutable_message()->set_steam_id("0110000149894186");
                PushMessage.mutable_message()->mutable_sign_info()->set_player_id(Sign->PlayerId);
                PushMessage.mutable_message()->mutable_sign_info()->set_sign_id(Sign->SignId);
                PushMessage.mutable_message()->set_player_struct((const char*)Sign->PlayerStruct.data(), Sign->PlayerStruct.size());

                if (!Client->MessageStream->Send(&PushMessage))
                {
                    Warning("[%s] Failed to send PushRequestSummonSign.", Client->GetName().c_str());
                }
            }
        }

        Timer = GetSeconds();
    }*/
}

MessageHandleResult SignManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetSignList)
    {
        return Handle_RequestGetSignList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateSign)
    {
        return Handle_RequestCreateSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRemoveSign)
    {
        return Handle_RequestRemoveSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateSign)
    {
        return Handle_RequestUpdateSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSummonSign)
    {
        return Handle_RequestSummonSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectSign)
    {
        return Handle_RequestRejectSign(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetRightMatchingArea)
    {
        return Handle_RequestGetRightMatchingArea(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool SignManager::CanMatchWith(const Frpg2RequestMessage::MatchingParameter& Client, const Frpg2RequestMessage::MatchingParameter& Match)
{
    // TODO: Actually apply some matching rules here.

    return true;
}

MessageHandleResult SignManager::Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetSignList* Request = (Frpg2RequestMessage::RequestGetSignList*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestGetSignListResponse Response;

    uint32_t RemainingSignCount = Request->max_signs();

    // Grab as many recent signs as we can from the cache that match our matching criteria.
    for (int i = 0; i < Request->search_areas_size() && RemainingSignCount > 0; i++)
    {
        const Frpg2RequestMessage::SignDomainGetInfo& Area = Request->search_areas(i);

        // Grab list of signs the client already has.
        std::unordered_set<uint32_t> ClientExistingSignId;
        for (int j = 0; j < Area.already_have_signs_size(); j++)
        {
            ClientExistingSignId.insert(Area.already_have_signs(i).sign_id());
        }

        OnlineAreaId AreaId = (OnlineAreaId)Area.online_area_id();
        uint32_t MaxForArea = Area.max_signs();
        uint32_t GatherCount = std::min(MaxForArea, RemainingSignCount);

        std::vector<std::shared_ptr<SummonSign>> AreaSigns = LiveCache.GetRecentSet(AreaId, GatherCount, [&Player, &Request](const std::shared_ptr<SummonSign>& Sign) { 
            return CanMatchWith(Request->matching_parameter(), Sign->MatchingParameters); 
        });
        for (std::shared_ptr<SummonSign>& Sign : AreaSigns)
        {
            // Filter players own signs.
            if (Sign->PlayerId == Player.PlayerId)
            {
                continue;
            }

            Log("[%s] Returning sign %i in area %i.", Client->GetName().c_str(), Sign->SignId, Sign->OnlineAreaId);

            // If client already has sign data we only need to return a limited set of data.
            if (ClientExistingSignId.count(Sign->SignId) > 0)
            {
                Frpg2RequestMessage::SignInfo* SignInfo = Response.mutable_get_sign_result()->add_sign_info_without_data();
                SignInfo->set_player_id(Sign->PlayerId);
                SignInfo->set_sign_id(Sign->SignId);
            }
            else
            {
                Frpg2RequestMessage::SignData* SignData = Response.mutable_get_sign_result()->add_sign_data();
                SignData->mutable_sign_info()->set_player_id(Sign->PlayerId);
                SignData->mutable_sign_info()->set_sign_id(Sign->SignId);
                SignData->set_online_area_id((uint32_t)Sign->OnlineAreaId);
                SignData->mutable_matching_parameter()->CopyFrom(Sign->MatchingParameters);
                SignData->set_player_struct(Sign->PlayerStruct.data(), Sign->PlayerStruct.size());
                SignData->set_steam_id(Sign->PlayerSteamId);
                SignData->set_is_red_sign(Sign->IsRedSign);
            }

            RemainingSignCount--;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetSignListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestCreateSign* Request = (Frpg2RequestMessage::RequestCreateSign*)Message.Protobuf.get();

    // DEBUG DEBUG DEBUG
    // Make a fake secondary sign as well from "another" player.
    /*{
        std::shared_ptr<SummonSign> DebugSign = std::make_shared<SummonSign>();
        DebugSign->SignId = NextSignId++;
        DebugSign->OnlineAreaId = (OnlineAreaId)Request->online_area_id();
        DebugSign->PlayerId = 1;
        DebugSign->PlayerSteamId = "0110000149894186";
        DebugSign->IsRedSign = Request->is_red_sign();
        DebugSign->PlayerStruct.assign(Request->player_struct().data(), Request->player_struct().data() + Request->player_struct().size());

        Ensure(DebugSign->PlayerStruct[452] == 'a');
        DebugSign->PlayerStruct[452] = 'b';
        DebugSign->MatchingParameters = Request->matching_parameter();
        LiveCache.Add(DebugSign->OnlineAreaId, DebugSign->SignId, DebugSign);
    }*/
    // DEBUG DEBUG DEBUG

    std::shared_ptr<SummonSign> Sign = std::make_shared<SummonSign>();
    Sign->SignId = NextSignId++;
    Sign->OnlineAreaId = (OnlineAreaId)Request->online_area_id();
    Sign->PlayerId = Player.PlayerId;
    Sign->PlayerSteamId = Player.SteamId;
    Sign->IsRedSign = Request->is_red_sign();
    Sign->PlayerStruct.assign(Request->player_struct().data(), Request->player_struct().data() + Request->player_struct().size());
    Sign->MatchingParameters = Request->matching_parameter();

    // Remove any existing summons signs, it should have already been done, but sometimes 
    // the client doesn't request to remove them during summon sign failures.
    /*for (std::shared_ptr<SummonSign>& Sign : Client->ActiveSummonSigns)
    {
        Warning("[%s] Forcibly removing existing summon sign during RequestCreateSign.", Client->GetName().c_str());
        LiveCache.Remove(Sign->OnlineAreaId, Sign->SignId);
    }
    Client->ActiveSummonSigns.clear();
    */

    LiveCache.Add(Sign->OnlineAreaId, Sign->SignId, Sign);
    Client->ActiveSummonSigns.push_back(Sign);

    Frpg2RequestMessage::RequestCreateSignResponse Response;
    Response.set_sign_id(Sign->SignId);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetSignListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRemoveSign* Request = (Frpg2RequestMessage::RequestRemoveSign*)Message.Protobuf.get();

    std::shared_ptr<SummonSign> Sign = LiveCache.Find((OnlineAreaId)Request->online_area_id(), Request->sign_id());
    if (!Sign)
    {
        Warning("[%s] Disconnecting client as attempted to remove non-existant summon sign, %i.", Client->GetName().c_str(), Request->sign_id());
        return MessageHandleResult::Error;
    }

    if (auto Iter = std::find(Client->ActiveSummonSigns.begin(), Client->ActiveSummonSigns.end(), Sign); Iter != Client->ActiveSummonSigns.end())
    {
        Client->ActiveSummonSigns.erase(Iter);
    }
    else
    {
        Warning("[%s] Disconnecting client as attempted to remove summon sign that didn't belong to them, %i.", Client->GetName().c_str(), Request->sign_id());
        return MessageHandleResult::Error;
    }

    LiveCache.Remove((OnlineAreaId)Request->online_area_id(), Request->sign_id());

    // Note: There is a response type for this message, but the server never normally sends it.

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestUpdateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdateSign* Request = (Frpg2RequestMessage::RequestUpdateSign*)Message.Protobuf.get();

    // I think the game uses this as something of a hearbeat to keep the sign active in the pool.
    // We keep all players signs active until they are removed, so we don't need to handle this.

    // Note: There is a response type for this message, but the server never normally sends it.

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestSummonSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestSummonSign* Request = (Frpg2RequestMessage::RequestSummonSign*)Message.Protobuf.get();

    // First check the sign still exists, if it doesn't, send a reject message as its probably already used.
    /*std::shared_ptr<SummonSign> Sign = LiveCache.Find((OnlineAreaId)Request->online_area_id(), Request->sign_info().sign_id());
    if (!Sign)
    {
        Warning("[%s] Client attempted to use invalid summon sign, sending back rejection, %i.", Client->GetName().c_str(), Request->sign_info().sign_id());

        Frpg2RequestMessage::PushRequestRejectSign PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectSign);
        PushMessage.mutable_message().

        if (!Client->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestRejectSign.", Client->GetName().c_str());
            return MessageHandleResult::Failed;
        }


        return MessageHandleResult::Handled;
    }*/


    // Send an accept message to the source of the sign.

    // TODO: 

    //

    // Note: There is a response type for this message, but the server never normally sends it.

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestRejectSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRejectSign* Request = (Frpg2RequestMessage::RequestRejectSign*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRejectSignResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectSignResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult SignManager::Handle_RequestGetRightMatchingArea(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetRightMatchingArea* Request = (Frpg2RequestMessage::RequestGetRightMatchingArea*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetRightMatchingAreaResponse Response;

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetRightMatchingArea response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string SignManager::GetName()
{
    return "Signs";
}
