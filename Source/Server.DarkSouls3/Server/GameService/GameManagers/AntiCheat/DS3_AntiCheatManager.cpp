/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatManager.h"
#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ClientFlagged.h"
#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ImpossibleStats.h"
#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_InvalidName.h"
#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_Exploit.h"

#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DebugTimer.h"
#include "Shared/Core/Utils/DebugObjects.h"

DS3_AntiCheatManager::DS3_AntiCheatManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
    Triggers.push_back(std::make_shared<DS3_AntiCheatTrigger_ClientFlagged>(this, InServerInstance, InGameServiceInstance));
    Triggers.push_back(std::make_shared<DS3_AntiCheatTrigger_Exploit>(this, InServerInstance, InGameServiceInstance));
    Triggers.push_back(std::make_shared<DS3_AntiCheatTrigger_ImpossibleStats>(this, InServerInstance, InGameServiceInstance));
    Triggers.push_back(std::make_shared<DS3_AntiCheatTrigger_InvalidName>(this, InServerInstance, InGameServiceInstance));
}

void DS3_AntiCheatManager::Poll()
{
    DebugTimerScope Scope(Debug::AntiCheatTime);

    GameManager::Poll();

    const RuntimeConfig& Config = ServerInstance->GetConfig();
    if (!Config.AntiCheatEnabled)
    {
        return;
    }

    // Scan through players and flag as appropriate.
    for (auto& Player : GameServiceInstance->GetClients())
    {
        PlayerState& State = Player->GetPlayerState();
        PlayerAntiCheatState& AntiCheatState = State.GetAntiCheatState_Mutable();

        if (State.GetPlayerId() >= 0 &&
            State.GetCharacterId() >= 0)
        {
            if (!AntiCheatState.HasLoadedPenalty)
            {
                AntiCheatState.Penalty = ServerInstance->GetDatabase().GetAntiCheatPenaltyScore(State.GetSteamId());
                AntiCheatState.HasLoadedPenalty = true;
            }

            // Scan for cheats.
            for (auto& Trigger : Triggers)
            {
                // Do not flag for the same trigger multiple times in the same connection.
                if (std::find(AntiCheatState.TriggersThisSession.begin(), AntiCheatState.TriggersThisSession.end(), Trigger->GetName()) != AntiCheatState.TriggersThisSession.end())
                {
                    continue;
                }

                std::string ExtraInfo = "";
                if (Trigger->Scan(Player, ExtraInfo))
                {
                    AntiCheatState.Penalty += Trigger->GetPenaltyScore();
                    AntiCheatState.ShouldApplyPenalty = true;
                    AntiCheatState.TriggersThisSession.push_back(Trigger->GetName());

                    LogS(Player->GetName().c_str(), "Player has been flagged for cheating by trigger '%s', penalty score is now %.2f: %s", Trigger->GetName().c_str(), AntiCheatState.Penalty, ExtraInfo.c_str());

                    ServerInstance->GetDatabase().AddAntiCheatPenaltyScore(State.GetSteamId(), AntiCheatState.Penalty);
                    ServerInstance->GetDatabase().LogAntiCheatTrigger(State.GetSteamId(), Trigger->GetName(), AntiCheatState.Penalty, ExtraInfo);

                    if (Config.SendDiscordNotice_AntiCheat && Config.AntiCheatApplyPenalties)
                    {
                        ServerInstance->SendDiscordNotice(Player, DiscordNoticeType::AntiCheat, StringFormat("Flagged for cheating: %s", ExtraInfo.c_str()));
                    }
                }
            }
        }
    }

    if (Config.AntiCheatApplyPenalties)
    {
        // Apply penalties to any users who are causing issues.
        for (auto& Player : GameServiceInstance->GetClients())
        {
            PlayerState& State = Player->GetPlayerState();
            PlayerAntiCheatState& AntiCheatState = State.GetAntiCheatState_Mutable();

            if (AntiCheatState.ShouldApplyPenalty)
            {
                std::string WarningMessage = "";

                if (AntiCheatState.Penalty > Config.AntiCheatBanThreshold)
                {
                    // Ban and disconnect.
                    ServerInstance->GetDatabase().BanPlayer(State.GetSteamId());
                    Player->DisconnectTime = GetSeconds() + 10.0f;

                    WarningMessage = Config.AntiCheatBanMessage;
                    AntiCheatState.ShouldApplyPenalty = false;

                    if (Config.SendDiscordNotice_AntiCheat)
                    {
                        ServerInstance->SendDiscordNotice(Player, DiscordNoticeType::AntiCheat, "Banned for cheating.");
                    }
                }
                else if (AntiCheatState.Penalty > Config.AntiCheatDisconnectThreshold)
                {
                    // Disconnect.
                    Player->DisconnectTime = GetSeconds() + 10.0f;

                    WarningMessage = Config.AntiCheatDisconnectMessage;
                    AntiCheatState.ShouldApplyPenalty = false;

                    if (Config.SendDiscordNotice_AntiCheat)
                    {
                        ServerInstance->SendDiscordNotice(Player, DiscordNoticeType::AntiCheat, "Disconnected for cheating.");
                    }
                }
                else if (AntiCheatState.Penalty > Config.AntiCheatWarningThreshold)
                {
                    // Send a warning.
                    WarningMessage = Config.AntiCheatWarningMessage;

                    // Don't reset penalty flag, we will keep sending the message during this session.
                }

                if (Config.AntiCheatSendWarningMessageInGame)
                {
                    if (GetSeconds() > AntiCheatState.WarningMessageCooldown)
                    {
                        Player->SendTextMessage(WarningMessage);
                        AntiCheatState.WarningMessageCooldown = GetSeconds() + Config.AntiCheatSendWarningMessageInGameInterval;
                    }
                }
            }
        }
    }
}

std::string DS3_AntiCheatManager::GetName()
{
    return "Anti-cheat";
}
