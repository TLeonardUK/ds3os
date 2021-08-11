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
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Loader
{
    // List of server configurations that can be converted freely to and from json.
    [Serializable]
    public class ServerConfigList
    {
        public List<ServerConfig> Servers { get; set; } = new List<ServerConfig>();

        public string ToJson()
        {
            return JsonSerializer.Serialize(this);
        }

        public static bool FromJson(string json, out ServerConfigList config)
        {
            try
            {
                config = JsonSerializer.Deserialize<ServerConfigList>(json);
                return true;
            }
            catch (JsonException)
            {
                config = new ServerConfigList();
                return false;
            }
        }
    }

    // Server configuration, stores the needed information required to connect to a 
    // given open server. Can be freely converted to/from json.
    [Serializable]
    public class ServerConfig
    {
        public string Name          { get; set; }
        public string Description   { get; set; }
        public string Hostname      { get; set; }
        public string PublicKey     { get; set; }

        public string ToJson()
        {
            return JsonSerializer.Serialize(this);
        }

        public static bool FromJson(string json, out ServerConfig config)
        {
            try
            {
                config = JsonSerializer.Deserialize<ServerConfig>(json);
                return true;
            }
            catch (JsonException)
            {
                config = new ServerConfig();
                return false;
            }
        }
    }
}
