# LoL_VPVC Proximity Chat for League (WIP)
A real-time Proximity Voice Chat for League of Legends. Built with C++ and Node.js, it uses OpenCV to track player positions on the minimap and WebSockets to transmit spatial audio. Voice volume fades dynamically based on in-game distance. Safe, standalone, and requires no game file modifications!

Inspired by the famous VPVC (Valorant Proximity Voice Chat), I decided to bring the same immersive and hilarious experience to the Summoner's Rift. Built entirely from scratch in just 3 days by a Brazilian developer 🇧🇷, this project is a standalone proximity voice chat for League of Legends.
It runs entirely in the background, consuming minimal resources, and does not modify any game files or memory, making it completely safe to use.

HOW IT WORKS

This project combines Computer Vision, Real-Time Networking, and Low-Level Audio Processing to create spatial audio based on your in-game location.
Computer Vision (OpenCV): The C++ client constantly captures your screen and reads the LoL minimap. It isolates the white camera bounding box to pinpoint your exact (X, Y) coordinates on the map.
Real-Time Network (Node.js & WebSockets): Your coordinates and raw microphone audio bytes are continuously packed and streamed to a Node.js signaling server.
Spatial Audio (Miniaudio): When your client receives your partner's audio and position, it calculates the geometric distance between both players. If the partner is close, the volume is at 100%. As they move away, the volume dynamically fades out until they are completely muted.

INSTALLATION & SETUP 
To play with a friend, one of you will need to host the server (the Host), and both will run the client.

Prerequisites
Node.js installed (for the Host).
Ngrok installed and have an account(for the Host, to allow internet connections).
The release package containing lol_proximity.exe and the OpenCV .dll file.

Step-by-Step Guide
1. Start the Server (Host Only)
Open a terminal in the folder containing server.js.

Run the following command to start the WebSocket server:

node server.js

Leave this terminal open in the background.

2. Create the Multiplayer Tunnel (Host Only)
To allow your friend to connect to your local server, we will use Ngrok to create a secure TCP tunnel.

Open a new terminal where your ngrok.exe is located.
Run the following command:

ngrok tcp 8080

Look for the Forwarding line in the Ngrok console. It will generate a link looking like tcp://0.tcp.sa.ngrok.io:12345. Copy this link and send it to your friend!
IMPORTANT: CHANGE THE 'TCP' WITH 'WS', OR IT WILL NOT WORK. It should look like this: tcp://0.tcp.sa.ngrok.io:12345 -> ws://0.tcp.sa.ngrok.io:12345

3. Run the Client (Both Players)
Make sure both players have the folder containing lol_proximity.exe and opencv_world4xx.dll (Without the DLL, the app won't open).
Double-click lol_proximity.exe.
A command prompt will ask for the server link.
Paste the Ngrok link the Host provided, but change tcp:// to ws://.
(Example: ws://0.tcp.sa.ngrok.io:12345)

Note: If you are playing alone on the Host PC and just want to test it, simply press ENTER without typing anything to connect to localhost

For now, the client is going to be in PT/BR, because i am going to use this with my f
