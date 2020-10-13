# Battleship
Have you ever wanted to play crappy battleship games with your friends over the internet in a console where both of you probably have to be using Linux?
Well then I've got a deal for you!

After compiling main.cpp, run it and you'll see a '>' symbol. Here's the help page:

help:
  Shows help page.

quit:
  Exits the application.

server <port>:
  Creates a server on the specified port and waits until a client attempts to connect.

connect <ip> <port>:
  Attempt to connect to a server running on the given IP address, over the given port.

When a server is created and a client connects, a game with randomized ship positions for each player is created and they take turns firing, starting with the server, until one player quits or wins.

Remember to port forward if connecting over the internet.
