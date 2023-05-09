# Chat-Room
Chat room application on a single computer where you and your (imaginary) friends can talk to each other. This application is created using UDT sockets. This application has the following functionality:

1. Server program that is always running on a specific port (for example, 5001).
2. Client program that can join this server.
3. The client needs a display name and a passcode to enter the chat room (assuming all
clients use the same passcode but different display name).
4. The job of the server is to accept connections from clients, get their display name and
passcode (in plaintext), verify that the passcode is correct and then allow clients into the
chat room.
5. When any client sends a message, the display name is shown before the message, and
the message is delivered to all other current clients.
6. Clients can type any text message, or can type one of the following shortcut codes to display specific text to everyone:
    - Type :Exit to close your connection and terminate the client
    - Type :) to display [feeling happy]
    - Type :( to display [feeling sad]
    - Type :mytime to display the current time
    - Type :+1hr to display the current time + 1 hour


## Connection Establishment and Password Checking
Each program takes the following CLI parameters: the client takes the server's IP and listening port, the username, and the password (all clients should use the same password). The server takes its listening port and the password.

If the password is not correct, the client receives a failure message "Incorrect passcode". Whenever a new client joins the chatroom, all other clients should receive a message indicating the username of the new user that has just joined the room.

**Example** 
1. Start the server
```zsh
~:$ ./chatserver -port 5001 -passcode <passcode> 
Server started on port 5001. Accepting connections...
```
2. Start the client
```zsh
~:$ ./chatclient -join -host <hostname> -username<username> \
                       -passcode <passcode> -port 5001
Connected to <hostname> on port 5001
```

Producing this output on the server the server

```zsh
<username> joined the chatroom
```

The server should be able to handle multiple clients connecting to it. This means that by running the above client command again (with a different username), the server should perform similarly. The server should also inform the already-connected clients that a new client has joined.

**Example**
1. On the new client client
```zsh
~:$ ./chatclient -join -host <hostname> -port 5001 \ -username<username> -passcode <passcode>
Connected to <hostname> on port 5001
```
2. On the server
```zsh
<username> joined the chatroom
```
3. On all other-connected clients
```zsh
<username> joined the chatroom
```

## Chat Functionality

After successfully connecting to the server, clients should be able to type messages that get sent to the server when the user enters a newline. All text before the newline should be sent to the server, displayed on the server's screen, and broadcasted and displayed on the screens of all clients.

**Example**
1. <username> client inputs “Hello Room” and its output should be
```zsh
  <username>: Hello Room
```
2. On the server the output should be
```zsh
  <username>: Hello Room
```
3. On all other clients, this should be the output
```zsh
  <username>: Hello Room
```
    
The same should work for chat shortcuts. For instance, if the user inputs “:)” then the server should output [Feeling Happy]. 
