![Dark Souls 3 - Open Server](https://github.com/TLeonardUK/ds3os/blob/main/Resources/banner.png?raw=true)

# What is this project?
An open source implementation of the dark souls 3 game server. 

Idealistically made for the purpose of allow better alternatives to playing mods than getting your account banned and using the retail ban server. As well as opening up opportunities to improve player safety and choice, by allowing them to segregate themselves off from the pool of cheaters on retail, without loosing network functionality.

:bangbang: This project is still a work in progress, no guarantees are given over its stability (that said it works quite well now!).

![Build Status](https://github.com/TLeonardUK/ds3os/actions/workflows/ci.yml/badge.svg)

# Where can I download it?
Downloads are available on the github releases page - https://github.com/TLeonardUK/ds3os/releases

# How do I use it?
Once built you should have a folder called Bin, there are 2 subfolders of relevance. Loader and Server. 

First run the Server.exe in Server, this will start the actual custom server running on your computer. 

The first time the server runs it will emit the file Saved\config.json which contains various matchmaking parameters that you can adjust (and apply by restarting the server) to customise the server.

User can join the server by opening the Loader.exe program in the Laoder folder and finding the correct server in the list and clicking the launch button. If the server has been configured to not advertise on the master server (or if running on a LAN without an internet connection), then the server operator can distribute the Saved\server.ds3osconfig file that the server emits, which can be imported into the loader and used to directly connect to the server.

Servers can also be password protected if required by setting as password in Saved\config.json, a password will need to be entered when attempting to launch the game with a protected server.

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
- [x] Master server support for loader (show available servers).
- [ ] WebUI for server showing gameplay statistics / allowing admin control.

# Will this ban my account on the retail server?
So far we've had several accounts using unoffical servers, for quite a while, and have not had any account penalized on the retail server.

We don't make any guarantees, but it seems safe enough.

# How do I build it?
Currently the project uses visual studio 2019 and C++17 for compilation, and as such is currently limited to windows. At some point in future the codebase will likely be moved over to something platform agnostic like cmake.

Ensure that you have vcpkg (https://vcpkg.io) installed and integrated into visual studio as well, as its usedfor managing a few of the dependencies, by doing the following:

1. Clone the vcpkg repo: `git clone https://github.com/Microsoft/vcpkg.git` or download the repository as .zip file
2. Run Windows PowerShell as Administrator and `cd` into the directory, example: `cd C:\dev\vcpkg`
3. Enter command: `.\bootstrap-vcpkg.bat` and wait for it to process
4. Enter command: `.\vcpkg integrate install` and wait for it to process

Building the project should now require opening the Source/DS3OpenServer.sln and building it.

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
