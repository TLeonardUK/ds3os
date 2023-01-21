/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Loader
{
    // Holds the information required to patch a given version of the dark souls exe.
    public struct DarkSoulsLoadConfig
    {
        // User facing description of the exe (eg. "Steam 0.0.1")
        public string VersionName;

        // The offset within exe module of the TEA encrypted payload that stores the server hostname and public key.
        public ulong ServerInfoAddress;

        // If true the game uses ASLR and needs to be patched relative to the module
        // base address.
        public bool UsesASLR;

        // If true the injector DLL will be loaded into the DS3 process rather than
        // patching the memory.
        public bool UseInjector;

        // TEA key used to encrypt payload.
        public uint[] Key;
    }

    // Holds compile time configuration.
    public static class BuildConfig
    {
        // Appid of steam version of dark souls 3. Used to create steam_appid.txt file to prevent steam starting on game launch.
        public static int SteamAppId = 374320;

        // Map of exe version to configuration required to load the exe.
        public static Dictionary<string, DarkSoulsLoadConfig> ExeLoadConfiguration = new Dictionary<string, DarkSoulsLoadConfig>()
        {
            { 
                ExeUtils.MakeSimpleExeHash("1.15.0.0", 102494368), 
                new DarkSoulsLoadConfig 
                { 
                    VersionName = "1.15.0.0 (Steam)", 
                    ServerInfoAddress = 0x144F4A5B1,
                    UseInjector = true,
                    UsesASLR = false,
                    Key = new uint[4] { 0x4B694CD6, 0x96ADA235, 0xEC91D9D4, 0x23F562E5 },
                } 
            },
            { 
                ExeUtils.MakeSimpleExeHash("1.15.1.0", 88982096), 
                new DarkSoulsLoadConfig 
                { 
                    VersionName = "1.15.1.0 (Steam)", 
                    ServerInfoAddress = 0x55A3F15,
                    UseInjector = true,
                    UsesASLR = true,
                    Key = new uint[4] { 0x970F4CFB, 0x1AA625DD, 0x172EBF85, 0x119A5426 },
                } 
            },
            { 
                ExeUtils.MakeSimpleExeHash("1.15.2.0", 88960032), 
                new DarkSoulsLoadConfig 
                { 
                    VersionName = "1.15.2.0 (Steam)", 
                    ServerInfoAddress = 0x0,
                    UseInjector = true,
                    UsesASLR = true,
                    Key = new uint[0] { },
                } 
            },
        };
    }
}

