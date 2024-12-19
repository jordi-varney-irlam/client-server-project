#client-server-project

# Client-Server Project (School Project)

This C project implements a basic chat application using a client-server model. The server listens for incoming connections and handles communication between multiple clients. The client connects to the server and allows users to send and receive messages in real-time.

## Overview

### Client
The client connects to the server over a TCP connection, sends messages, and receives messages from the server. The client operates in non-blocking mode, allowing it to simultaneously send and receive messages without being interrupted by I/O operations. The client reads input from `stdin` and sends it to the server. It also continuously checks for incoming messages from the server using polling and prints them to `stdout`.

### Server
The server accepts incoming client connections and maintains a list of connected clients. It uses a non-blocking approach to handle multiple clients concurrently. When a message is received from any client, it is forwarded to all other connected clients. The server can handle up to four clients at a time. If the server is full, it rejects new clients with a message indicating that the room is full.

## Features
- Non-blocking I/O for both the client and server.
- Multiple clients can connect to the server and communicate with each other.
- The server forwards messages received from clients to all other connected clients.
- Clients and server use TCP connections over localhost (127.0.0.1) on port `4124`.
- A maximum of 4 clients can be connected to the server at any given time.

## Compilation and Building

To compile the project, you can use the provided `Makefile`. It defines targets for building both the server and the client.


