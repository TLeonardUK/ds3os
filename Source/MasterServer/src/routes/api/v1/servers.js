/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

const express = require('express');
const router = express.Router();

const config = require("../../../config/config.json")

var GActiveServers = [];

function RemoveServer(IpAddress)
{
    for (i = 0; i < GActiveServers.length; i++)
    {
        if (GActiveServers[i].ip_address == IpAddress)
        {
            GActiveServers.splice(i, 1);
            break;
        }
    }
}

function AddServer(IpAddress, hostname, description, name, public_key, player_count, password, mods_white_list, mods_black_list, mods_required_list)
{
    GActiveServers.push({
       "ip_address": IpAddress,
       "hostname": hostname,
       "description": description,
       "name": name,
       "public_key": public_key,
       "player_count": player_count,
       "password": password,
       "mods_white_list": mods_white_list,
       "mods_black_list": mods_black_list,
       "mods_required_list": mods_required_list,
       "updated_time": Date.now()
    });
}

function RemoveTimedOutServers()
{
    var TimeoutDate = new Date(Date.now() - config.server_timeout_ms);
    for (i = 0; i < GActiveServers.length; )
    {
        if (GActiveServers[i].updated_time < TimeoutDate)
        {
            console.log(`Removing server that timed out: ip=${GActiveServers[i].ip_address}`);

            GActiveServers.splice(i, 1);
            break;
        }
        else
        {
            i++;
        }
    }
}

// @route GET api/v1/servers
// @description Get a list of all active servers.
// @access Public
router.get('/', async (req, res) => {
    RemoveTimedOutServers();

    var ServerInfo = [];    
    for (i = 0; i < GActiveServers.length; i++)
    {
        ServerInfo.push({
            "ip_address": GActiveServers[i]["ip_address"],
            "hostname": GActiveServers[i]["hostname"],
            "description": GActiveServers[i]["description"],
            "name": GActiveServers[i]["name"],
            "player_count": GActiveServers[i]["player_count"],
            "password_required": GActiveServers[i]["password"].length > 0,
            "mods_white_list": GActiveServers[i]["mods_white_list"],
            "mods_black_list": GActiveServers[i]["mods_black_list"],
            "mods_required_list": GActiveServers[i]["mods_required_list"]
        });
    }

    res.json({ "status":"success", "servers": ServerInfo });
});

// @route GET api/v1/servers/:ip_address/public_key
// @description Get the public kley of a given server.
// @access Public
router.get('/:ip_address/public_key', async (req, res) => {
    if (!('password' in req.body))
    {
        res.json({ "status":"error", "message":"Expected password in body." });
        return;        
    }

    var password = req.body["password"];

    var ServerInfo = [];    
    for (i = 0; i < GActiveServers.length; i++)
    {
        if (GActiveServers[i].ip_address == req.params.ip_address)
        {
            if (password == GActiveServers[i].password)
            {
                res.json({ "status":"success", "public_key":GActiveServers[i].public_key });
            }
            else
            {
                res.json({ "status":"error", "message":"Password was incorrect." });
            }
            return;
        }
    }

    res.json({ "status":"error", "message":"Failed to find server." });
});

// @route POST api/v1/servers
// @description Adds or updates the server registered to the clients ip.
// @access Public
router.post('/', async (req, res) => {
    RemoveServer(req.connection.remoteAddress);

    if (!('hostname' in req.body))
    {
        res.json({ "status":"error", "message":"Expected hostname in body." });
        return;        
    }
    if (!('description' in req.body))
    {
        res.json({ "status":"error", "message":"Expected description in body." });
        return;        
    }
    if (!('name' in req.body))
    {
        res.json({ "status":"error", "message":"Expected name in body." });
        return;        
    }
    if (!('public_key' in req.body))
    {
        res.json({ "status":"error", "message":"Expected public_key in body." });
        return;        
    }
    if (!('player_count' in req.body))
    {
        res.json({ "status":"error", "message":"Expected player_count in body." });
        return;        
    }
    if (!('password' in req.body))
    {
        res.json({ "status":"error", "message":"Expected password in body." });
        return;        
    }
    if (!('mods_white_list' in req.body))
    {
        res.json({ "status":"error", "message":"Expected mods_white_list in body." });
        return;        
    }
    if (!('mods_black_list' in req.body))
    {
        res.json({ "status":"error", "message":"Expected mods_black_list in body." });
        return;        
    }
    if (!('mods_required_list' in req.body))
    {
        res.json({ "status":"error", "message":"Expected mods_required_list in body." });
        return;        
    }

    var hostname = req.body["hostname"];
    var description = req.body["description"];
    var name = req.body["name"];
    var public_key = req.body["public_key"];
    var player_count = parseInt(req.body["player_count"]);
    var password = req.body["password"];
    var mods_white_list = req.body["mods_white_list"];
    var mods_black_list = req.body["mods_black_list"];
    var mods_required_list = req.body["mods_required_list"];

    AddServer(req.connection.remoteAddress, hostname, description, name, public_key, player_count, password, mods_white_list, mods_black_list, mods_required_list);
    
    res.json({ "status":"success" });
});

// @route DELETE api/v1/servers
// @description Delete the server registered to the clients ip.
// @access Public
router.delete('/', (req, res) => { 
    RemoveServer(req.connection.remoteAddress);
    res.json({ "status":"success" });
});

module.exports = router;