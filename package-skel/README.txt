
# About

This package contains everything you need to launch a Neverwinter Nights 2
server and connect to it.

# Requirements

- Neverwinter Nights 2, with the latest patches
- Visual C++ 2005 x86 Runtime:
  https://download.microsoft.com/download/8/B/4/8B42259F-5D70-43F4-AC2E-4B208FD8D66A/vcredist_x86.EXE
- Visual C++ 2015 x86 Runtime:
  https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe
- .NET Framework 4.7.2 or above:
  https://download.visualstudio.microsoft.com/download/pr/1f5af042-d0e4-4002-9c59-9ba66bcf15f6/124d2afe5c8f67dfa910da5f9e3db9c1/ndp472-kb4054531-web.exe


# Launch the server

Double-click on `start-server.bat`. This will launch NWNX4_GUI and the
NWN2Server.

# Launch the game

Once the server is running, you can double-click either:
- `start-game-autoconnect.bat` to start the game and automatically connect to
  the server running on your machine (localhost)?
- `start-game.bat` to start the game. You will need to go to Multiplayer ->
  Direct connect, and type the IP address of the machine running your NWN2
  server.
