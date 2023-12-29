/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "Shared/Platform/Platform.h"

// Represents the in-game state of a player. Each game client owns an instance of this class.

#define DEFINE_FIELD(type, name, default_value)                             \
    private: type name = default_value;                                     \
    public:                                                                 \
        const type& Get##name() const { return name; }                      \
        type& Get##name##_Mutable() { return name; }                        \
        void Set##name(const type& input) { name = input; Mutated(); }   

struct PlayerAntiCheatState
{
    // The penalty score of the user, this increases as triggers occur.
    float Penalty = 0.0f;

    // If the players current anti-cheat penalty has been loaded from the database yet.
    bool HasLoadedPenalty = false;

    // If set to true, we should apply whatever penalty the user deserves 
    // on the next iteration.
    bool ShouldApplyPenalty = false;

    // What triggers have been activated this frame. We can only activate each one
    // once per session.
    std::vector<std::string> TriggersThisSession;

    // Cooldown between showing warning management messages.
    float WarningMessageCooldown = 0.0f;
    
    // Flagged by various parts of the code if the user has attempted a security exploit.
    bool ExploitDetected = false;
};

struct PlayerState
{
private:
    size_t Generation;

public:
    size_t GetGeneration() { return Generation; }
    void Mutated() { Generation++; }

    // Steam ID of logged in user as a hex string.
    DEFINE_FIELD(std::string, SteamId, "")    
    
    // Unique ID of the player account. Used for identifying the 
    // player in most packets past initial login, steam id is not used.
    DEFINE_FIELD(uint32_t, PlayerId, 0)
    
    // What character id they are currently playing.
    DEFINE_FIELD(int, CharacterId, -1)
    
    // The name of the character the player is currently playing with. Will be empty
    // until the first RequestUpdatePlayerStatus is invoked.
    DEFINE_FIELD(std::string, CharacterName, "")    
    
    // If the player is currently in a state they can be invaded in.
    DEFINE_FIELD(bool, IsInvadable, false)
    
    // Players current soul level.
    DEFINE_FIELD(int, SoulLevel, 0)
    
    // Players maximum weapon level. 
    DEFINE_FIELD(int, MaxWeaponLevel, 0)
        
    // Current anti-cheat penalty score.
    DEFINE_FIELD(PlayerAntiCheatState, AntiCheatState, {});

    // Bonfire info used for calculating deltas on when the user lights one.
    DEFINE_FIELD(std::vector<uint32_t>, LitBonfires, {});
    DEFINE_FIELD(bool, HasInitialState, false);

    // Gets a game specific opaque identifier for the area the player is in.
    virtual uint32_t GetCurrentAreaId() = 0;

    // Returns true when the player is fully logged in and in-game.
    virtual bool IsInGame() = 0;

    // Various bits of player information.
    virtual size_t GetSoulCount() = 0;
    virtual size_t GetSoulMemory() = 0;
    virtual size_t GetDeathCount() = 0;
    virtual size_t GetMultiplayerSessionCount() = 0;
    virtual double GetPlayTime() = 0;

    // Textural descriptions of the players status/covenant status, shown in the web-interface.
    virtual std::string GetConvenantStatusDescription() = 0;
    virtual std::string GetStatusDescription() = 0;

};

#undef DEFINE_FIELD