![Dark Souls 3 - Open Server](./Resources/banner.png?raw=true)

![GitHub license](https://img.shields.io/github/license/TLeonardUK/ds3os)
![GitHub release](https://img.shields.io/github/release/TLeonardUK/ds3os)
![GitHub downloads](https://img.shields.io/github/downloads/TLeonardUK/ds3os/total)
[![Discord](https://img.shields.io/discord/937318023495303188?label=Discord)](https://discord.gg/pBmquc9Jkj)

# What is this project?
An open source implementation of the game servers for dark souls 2 (SOTFS) and 3. 

This project exists to provide an alternative to playing online with mods without the risk of being banned, or just for people who want to play privately and not deal with cheaters/invasions/etc.

If you have any trouble join the discord for tech-support: https://discord.gg/pBmquc9Jkj

# Can I use it with a pirated game?
No, the server authenticates steam tickets. Please do not ask about piracy, steam emulators or the like, we have no interest in supporting them. 

FROM SOFTWARE deserves your support too for the excellent work they do, please buy their games if you can.

# Where can I download it?
Downloads are available on the github releases page - https://github.com/TLeonardUK/ds3os/releases

# How do I use it?
Once built you should have a folder called Bin, there are 2 subfolders of relevance. Loader and Server. 

When running the loader you will get the option to create a server or join an existing server.

If you want to create a dedicated server yourself you should run Server.exe in the Server folder, this will start the actual custom server running on your computer. 

The first time the server runs it will emit the file Saved\default\config.json which contains various matchmaking parameters that you can adjust (and apply by restarting the server) to customise the server.

Servers can also be password protected if required by setting as password in Saved\default\config.json, a password will need to be entered when attempting to launch the game with a protected server.

**NOTE**: The **steam** client (no login needed) must be installed when you run the **Server.exe**. Otherwise, **Server.exe** will fail to initialize.

# What currently works?
Most of the games core functionality works now, with some degree of variance to the retail game. We're currently looking to closer match retail server behaviour and make some general improvements to the running of unoffical servers.

:bangbang: Dark Souls 2 support is still experimental and under development, there is a high probability of things not behaving correctly.

| Feature | Dark Souls 3 | Dark Souls 2 SOTFS |
| --- | --- | --- |
| Stable enough for use | :heavy_check_mark: | Experimental |
| Network transport | :heavy_check_mark: | :heavy_check_mark:  |
| Announcement messages | :heavy_check_mark:  | :heavy_check_mark:  |
| Profile management | :heavy_check_mark:  | :heavy_check_mark:  |
| Blood messages | :heavy_check_mark: | :heavy_check_mark:  |
| Bloodstains | :heavy_check_mark: | :heavy_check_mark:  |
| Ghosts | :heavy_check_mark: | :heavy_check_mark:  |
| Summoning | :heavy_check_mark: | :heavy_check_mark: |
| Invasions | :heavy_check_mark: | :heavy_check_mark: |
| Auto-Summoning (Convenants) | :heavy_check_mark: | :heavy_check_mark: |
| Mirror Knight | n/a | :heavy_check_mark: |
| Matchmaking | :heavy_check_mark: | :heavy_check_mark: |
| Leaderboards | :heavy_check_mark: | :heavy_check_mark: |
| Bell Ringing | :heavy_check_mark: | n/a |
| Quick Matches (Arenas) | :heavy_check_mark: | :heavy_check_mark: |
| Telemetry/Misc | :heavy_check_mark: | :heavy_check_mark:  |
| Ticket Authentication | :heavy_check_mark: | :heavy_check_mark: |
| Master Server Support | :heavy_check_mark: | :heavy_check_mark: |
| Loader Support | :heavy_check_mark: | :heavy_check_mark: |
| WebUI For Admin | :heavy_check_mark: | :heavy_check_mark: |
| Sharding Support | :heavy_check_mark: | :heavy_check_mark: |
| Discord Activity Feed | :heavy_check_mark: |  |

Future roadmap:

- Support for various mod-settings per server (eg. allow servers to remove summon limit)
- Better Anticheat (potentially we could do some more harsh checks than FROM does).

# Will this ban my account on the retail server?
DSOS uses its own save files, as long as you don't copy ds3os saves back to your retail saves you should be fine.

# FAQ
# How do I switch between hosting Dark Souls 3 and Dark Souls 2?
After running the server once a file will be created at Saved/default/config.json. You can change the GameType parameter from DarkSouls2 and DarkSouls3 to change what game the server hosts.

## Why aren't my save files appearing?
DSOS uses its own saves to avoid any issues with retail game saves. If you want to transfer your retail saves to DSOS, click the settings (cog) icon at the bottom of the loader and press the copy retail saves button.

We don't provide an automation option to copy ds3os saves back to retail saves for safety. If you ~really~ want to do this you can find the folder the saves are stored in and rename the .ds3os files to .sl2.

## Can I run the server via docker?
Yes, there are 2 docker containers currently published for DSOS, these are automatically updated each time a new release is made:

timleonarduk/ds3os - This is the main server and the one you almost certainly want.
timleonarduk/ds3os-master - This is for the master server, unless you are making a fork of ds3os, you probably don't need this.

If you want a quick one-liner to run the server, you can use this. Note that it mounts the Saved folder to the host filesystem at /opt/ds3os/Saved, making it easier to modify the configuration files. Access /opt/ds3os/Saved to view and modify the configuration files.

`sudo mkdir -p /opt/ds3os/Saved && sudo chown 1000:1000 /opt/ds3os/Saved && sudo docker run -d -m 2G --restart always --net host --mount type=bind,source=/opt/ds3os/Saved,target=/opt/ds3os/Saved timleonarduk/ds3os:latest`

## I launch the game but its unable to connect?
There are a few different causes of this, the simplest one is to make sure you're running as admin, the launcher needs to patch the games memory to get it to connect to the new server, this requires admin privileges.

If the server is being hosted by yourself and the above doesn't solve your issue, try these steps:

1. Ensure these ports are forwarded on your router, both for tcp and udp: 50000, 50010, 50050, 50020 

2. Ensure you have allowed the server access through the windows defender firewall, you can set rules here: Start Bar -> Windows Administrative Tools -> Windows Defender Firewall with Advanced Security -> Inbound/Output Rules

3. Its possible you don't have the configuration for the server setup correctly. After running the server once make sure to open the configuration file (Saved/config.json) and make sure its setup correctly (it will attempt to autoconfigure itself, but may get incorrect values if you have multiple network adapters). The most critical settings to get correct are ServerHostname and ServerPrivateHostname, these should be set to your WAN IP (the one you get from sites like https://whatismyip.com), and your LAN IP (the one you get from running ipconfig) respectively. If you are using LAN emulation software (eg. hamachi) you will need to set these to the appropriate hamachi IP.

## What do all the properties in the config file mean?
The settings are all documented in the source code in this file, in future I'll write some more detailed documentation.

https://github.com/TLeonardUK/ds3os/blob/main/Source/Server/Config/RuntimeConfig.h

# How do I build it?
Currently the project uses visual studio 2022 and C++17 for compilation.

We use cmake for generating project files. You can either use the cmake frontend to generate the project files, or you can use one of the generate_* shell scripts inside the Tools folder.

Once generated the project files are stored in the intermediate folder, at this point you can just open them and build the project.

## Using nix

```sh
# to build a package
nix build github:TLeonardUK/ds3os
# to run it directly
nix run github:TLeonardUK/ds3os
# to run master-server
nix run github:TLonardUK/ds3os#master-server
```

The nix version stores the configs in `${XDG_CONFIG_HOME:-$HOME/.config}/ds3os`

# Whats in the repository?
```
/
├── Protobuf/              Contains the protobuf definitions used by the server's network traffic. Compiling them is done via the bat file in Tools/
├── Resources/             General resources used for building and packaging - icons/readmes/etc.
├── Source/                All source code for the project.
│   ├── Injector/          This is the DLL that gets injected into the game to provide DS3OS's functionality.
│   ├── Loader/            Simple winforms app that loads DS3 such that it will connect to a custom server.S3 such that it will connect to a custom server.
│   ├── MasterServer/      NodeJS source code for a simple API server for advertising and listing active servers.
│   ├── Server/            Source code for the main server.
│   ├── Server.DarkSouls3/ Source code thats special to dark souls 3 support.
│   ├── Server.DarkSouls2/ Source code thats special to dark souls 2 support.
│   ├── Shared/            Source code that is shared between the server and injector projects.
│   └── ThirdParty/        Source code for any third-party libraries used.
│   └── WebUI/             Contains the static resources used to assemble the management web page for the server.
├── Tools/                 Various cheat engine tables, bat files and alike used for analysis.
```

# How can I help?
Check our the issues page, or send me a message for suggestions on what can be done.

Right now there are a few server calls we either have stubbed out or returning dummy information, implementing
them properly, or finding out the format of the data they need to return would be worth while.

There are also a lot of protobuf fields that are still unknown and use constant values when sent from the 
server, determining what they represent would be a good improvement.

# Credit
A lot of the information needed to produce this implementation has been figured out by the community. 
Especially the members of the ?ServerName? souls modding discord.

The following 3 repositories have provided a lot of information used in this implementation:

https://github.com/garyttierney/ds3-open-re

https://github.com/Jellybaby34/DkS3-Server-Emulator-Rust-Edition

https://github.com/AmirBohd/ModEngine2

Graphics and icons provided by:

Campfire icon made by ultimatearm from www.flaticon.com

Various UI icons made by Mark James from http://www.famfamfam.com/lab/icons/silk/
