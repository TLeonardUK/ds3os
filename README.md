# Dark Souls 3 - Open Server
An open source implementation of the dark souls 3 game server. 

Idealistically made for the purpose of allow better alternatives to playing mods than getting your account banned and using the retail ban server. As well as opening up opportunities to improve player safety and choice, by allowing them to segregate themselves off from the pool of cheaters on retail, without loosing network functionality.

:bangbang: This project is still a work in progress, no guarantees are given over its stability (that said it works quite well now!).

![Build Status](https://github.com/TLeonardUK/ds3os/actions/workflows/ci.yml/badge.svg)

# Whats in the repository?
```
/
├── Config/               Contains any useful configuration material, server config files and alike.
├── Protobuf/             Contains the protobuf definitions used by the server's network traffic. Compiling them is done via the bat file in Tools/
├── Resources/            General resources used for building and packaging - icons/readmes/etc.
├── Source/               All source code for the project.
│   ├── Loader/           Simple winforms app that loads DS3 such that it will connect to a custom server.
│   ├── Server/           Source code for the main server.
│   └── ThirdParty/       Source code for any third-party libraries used. Preference is storing source here over using vcpkg modules where practical.
├── Tools/                Various cheat engine tables, bat files and alike used for analysis.
```

# Where can I download it?
Downloads are available on the github releases page - https://github.com/TLeonardUK/ds3os/releases

# How do I build it?
Currently the project uses visual studio 2019 and C++17 for compilation, and as such is currently limited to windows. At some point in future the codebase will likely
be moved over to something platform agnostic like cmake.

Building the project should just require opening Source/DS3OpenServer.sln and building it. Ensure that you have vcpkg (https://vcpkg.io) installed and integrated into 
visual studio as well, as its usedfor managing a few of the dependencies.

# How do I use it?
Once built you should have a folder called Bin, there are 2 subfolders of relevance. Loader and Server. 

First run the Server.exe in Server, this will start the actual custom server running on your computer. 

When it runs it should write out a file to Saved\server.ds3osconfig, this is a simple json file that contains a description of your server and the needed cryptographic keys
to join it, you can send this to people who you want to join your server.

While the server is running run the file Loader.exe in the Loader subfolder. This is a simple winform app that handles booting up the game in a way that it can 
join the custom server. Within this app set the game path to your DarkSoulsIII.exe and import the server.ds3osconfig file that was made by the server earlier. You should
now be able to click launch to start the game which will automatically connect to your new server.

# What currently works?
Most of the games core functionality works now, with some degree of variance to the retail game. We're currently looking to closer
match retail server behaviour and make some general improvements to the running of unoffical servers.

- [x] Login, key exchange and network transport
- [x] Announcement messages
- [x] Profile management
- [x] Blood messages
- [x] Bloodstains
- [x] Ghosts
- [x] Summoning
- [x] Invasions
- [x] Visitors (Auto-Summoning via covenant)
- [x] Matchmaking (eg. Correctly matching summoning/invasions/visits with player level)
- [x] Roster of knights
- [x] Archdragon peak bell ringing
- [x] Undead match
- [x] Telemetry and misc server calls (the few that are of use to us)

Future roadmap:

- [ ] Regulation file distribution (likely won't be implemented as it involves distributing copyrighted content)
- [ ] Anticheat (potentially we could do some more harsh checks than FROM does).
- [ ] Steam ticket authentication.
- [ ] Master server support for loader (show available servers).
- [ ] WebUI for server showing gameplay statistics / allowing admin control.

# Will this ban my account on the retail server?
So far we've had several accounts using unoffical servers, for quite a while, and have not had any account penalized on the retail server.

We don't make any guarantees, but it seems safe enough.

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
