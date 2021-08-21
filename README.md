# Dark Souls 3 - Open Server
An open source implementation of the dark souls 3 game server. Initially focused on getting matchmaking working such that private servers can be run.

:bangbang: This project is very much an early work in progress, no guarantees are given over its stability.

![Build Status](https://github.com/TLeonardUK/ds3os/actions/workflows/msbuild.yml/badge.svg)

# Whats in the repository?
```
/
├── Config/               Contains any useful configuration material, server config files and alike.
├── Protobuf/             Contains the protobuf definitions used by the server's network traffic. Compiling them is done via the bat file in Tools/
├── Source/               All source code for the project.
│   ├── Loader/           Simple winforms app that loads DS3 such that it will connect to a custom server.
│   ├── Server/           Source code for the main server.
│   └── ThirdParty/       Source code for any third-party libraries used. Preference is storing source here over using vcpkg modules where practical.
├── Tools/                Various cheat engine tables, bat files and alike used for analysis.
```

# Where can I download it?
No binaries are available at the moment. If you want to run it, you will have to build it yourself.

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
We're slowly going through and implementing all the games online functionality, this is the current state:

- [x] Login, key exchange and network transport
- [x] Announcement messages
- [x] Profile management
- [x] Blood messages
- [x] Bloodstains
- [x] Ghosts
- [ ] Roster of knights
- [ ] Invasions
- [ ] Summoning
- [ ] Visitors (Summoning via covenant)
- [ ] Undead match
- [ ] Archdragon peak bell ringing
- [ ] Telemetry and misc server calls
- [ ] Regulation file distribution (likely won't be implemented as it involves distributing copyrighted content)

# How can I help?
At the moment of lot of the general structure is being put together so you might want to hold trying to contibute any large changes right now.

However the one thing that would be very useful is figuring out all the unknown fields in the protobuf definitions and figure out what the currently hard-coded 
constants in some of the packet headers do.

# Credit
A lot of the information needed to produce this implementation has been figured out by the community. 
Especially the members of the ?ServerName? souls modding discord.

The following 3 repositories have provided a lot of information used in this implementation:

https://github.com/garyttierney/ds3-open-re

https://github.com/Jellybaby34/DkS3-Server-Emulator-Rust-Edition

https://github.com/AmirBohd/ModEngine2
