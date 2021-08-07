// Dark Souls 3 - Open Server

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
                    ServerInfoAddress = 0x144F4A5B1 
                } 
            },
        };
    }
}

