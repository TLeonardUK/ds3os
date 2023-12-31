-------------------------------------------------------------------------
 Dark Souls - Open Server
 https://github.com/TLeonardUK/ds3os
-------------------------------------------------------------------------

Before running any executables, please install the files in Prerequisites 
first, these are C++ runtimes required to run the executables, without 
them they may not run.

For up to date documentation please read the information on the github readme.
https://github.com/TLeonardUK/ds3os

Contained within are 2 folders:

-- Loader --
This tool allows you to launch Dark Souls 2/3 in a way that allows you to join 
an unofficial server. All you need to do is open the Loader, set the path 
to your game's exe path (normally somewhere in your steam directory).

Be warned, while it does not currently seem to occur, no guarantee is made that 
your account will not be soft-banned on offical servers if you use this tool.
It would be wise to use a family-share account or a similar disposable account 
when using this.

-- Server --
When run this will launch a dark souls 2/3 server. 

We do not advise people to use this directly, instead join an existing server 
or create a server from the loader, this will be much simpler and avoid needing
to handle port-forwarding/firewalls/etc.

If you really want to run a dedicated server yourself;- when launched it will save 
a file to Saved\default\config.json, this file contains configuration settings 
for the server users to join the server. You should open this file and modify 
the appropriate settings to your preferences.
If you do not have a hostname that points to your computer use your 
external IP address (the one you find from visiting sites like 
https://www.whatismyip.com/).

The server accepts connections on ports 50050, 50000 and 50010 by default. If 
the computer you are running the server on is behind a router you will likely
need to modify your router to port-forward TCP and UDP on all of those ports to
allow external people to connect. Guides to doing this are numerous and available
online.
