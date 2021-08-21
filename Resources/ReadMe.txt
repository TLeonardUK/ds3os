-------------------------------------------------------------------------
 Dark Souls 3 - Open Server
 https://github.com/TLeonardUK/ds3os
-------------------------------------------------------------------------

Contained in this folder are 2 executables.

-- Server.exe --
When run this will launch a dark souls 3 server. When launched it will save 
a file to Saved\server.ds3oconfig, this file contains configuration settings 
and cryptographic keys that allow other users to join the server. You should 
open this file and modify the Name, Description and Hostname settings your
server. If you do not have a hostname that points to your computer use your 
external IP address (the one you find from visiting sites like 
https://www.whatismyip.com/).

The server accepts connections on ports 50050, 50000 and 50010 by default. If 
the computer you are running the server on is behind a router you will likely
need to modify your router to port-forward TCP and UDP on all of those ports to
allow external people to connect. Guides to doing this are numerous and available
online.

-- Loader.exe --
This tool allows you to launch Dark Souls III in a way that allows you to join 
an unofficial server. All you need to do is open the Loader, set the path 
to your DarkSoulsIII.exe (normally somewhere in your steam directory) and then
import a .ds3oconfig file that describes the server to connect to, and should have
been given to you by whoever is running the server.

Be warned, while it does not currently seem to occur, no guarantee is made that 
your account will not be soft-banned on offical servers if you use this tool.
It would be wise to use a family-share account or a similar disposable account 
when using this.