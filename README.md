# Multiroom chat application

## Short Description

This project implements a cartoon chat where users can communicate in different rooms. The application uses a client-server architecture where clients connect to the server and exchange messages in real time.

## The main functions and features of the app include:

1. **Creating and joining rooms**: Users can create new chat rooms or join existing ones. 

2. **Rejoining rooms**: Users can change the room they are chatting in 

3. **Messaging**: Users can send text messages within the chat room they are in.

4. **Real-time**: Messaging takes place in real time, allowing users to see other participants' messages almost instantly after they are sent.
   
## Interaction Process:

The server initializes on port 12348 with the IP Address 127.0.0.1 (IPv4, AF_INET), utilizing TCP (SOCK_STREAM). Each char transmitted corresponds to 1 byte in C++.

## Client Interaction Process:

### Initialization:

1. The client initializes by specifying the server's IP address and port number.
2. It creates a socket using the `socket()` system call, specifying the address family as IPv4 (AF_INET) and the communication type as TCP (SOCK_STREAM).
3. The client connects to the server using the `connect()` system call, providing the server's address and port.

### Sending User Information:

1. The client prompts the user to enter their name and the room ID they wish to join.
2. It sends this information to the server using the `send()` system call.

### Sending and Receiving Messages:

1. The client enters a loop to continuously send and receive messages.
2. It prompts the user to input a message.
3. If the message is "STOP," the client breaks out of the loop and exits.
4. If the message starts with "REJOIN," the client sends it to the server indicating the intention to join a different room.
5. Otherwise, it sends the message to the server.
6. The client receives messages from the server using the `recv()` system call and displays them to the user.

## Server-side Interactive Process:

### Initialization:

1. The server initializes on port 12348 with the IP Address 127.0.0.1 (IPv4, AF_INET), utilizing TCP (SOCK_STREAM) for communication.
2. It creates a socket using the `socket()` system call, specifying the address family as IPv4 (AF_INET) and the communication type as TCP (SOCK_STREAM).
3. The server binds the socket to its address using the `bind()` system call and listens for incoming connections using the `listen()` system call.

### Accepting Client Connections:

1. The server enters a loop to continuously accept client connections.
2. When a client connects, the server accepts the connection using the `accept()` system call, creating a new socket for communication with that client.
3. It spawns a new thread or process to handle communication with the client separately.

### Handling Client Requests:

1. Each client handler thread/process receives the user information (name and room ID) sent by the client.
2. It adds the client to the specified room or creates a new room if it does not exist.
3. The server then enters a loop to continuously receive messages from the client.
4. If a message is received, it broadcasts the message to all clients in the same room except the sender.
5. If the message is "STOP," indicating the client wants to disconnect, it removes the client from the room and closes the connection.
6. If the message starts with "REJOIN," indicating the client wants to join a different room, it processes the request accordingly.

### Closing Connections:

1. When a client disconnects, the server closes the corresponding socket and removes the client from the room.


**Compilation and Execution Requirements**

- To compile the server: `g++ server.cpp -o server`
- To run the server: `./server`

- To compile the client: `g++ client.cpp -o client`
- To run the client: `./client`
  
## Server side 
![server_side](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/d0a6d446-4c0d-4208-9bdd-1ac6137b2087)

## Clients side
![client1_side](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/0494a4ba-7cf3-44b8-ae1c-d623f0d84421)

![client2_side](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/0e156b7b-d3dc-4f8f-aed4-0e229464d5b4)

![client3_side](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/e0c6dd4d-83f5-4256-8f4d-d5a6179668b7)


## Explanation with UML diagrams 

1. *Class Diagram*
![class_diagram](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/81442395-a9cf-4a4f-9fd9-55227e7cb11f)

2. *Use Case Diagram* 

![use_case_diagram](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/d7b2ecbd-a48c-4161-82c0-dc58aa28a097)

3. *Sequence Diagram*

![sequence_diagram](https://github.com/Anastasiiasyvak/multi-room_chat_application/assets/119412566/1cbf1779-b94b-4ff3-a3d0-a16ff629edc7)
