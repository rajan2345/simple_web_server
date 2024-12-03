 
HIGH-LEVEL STEPS TO CREATE A TCP SERVER:
1. Create a Listening Socket: 
     A socket that listens for incoming client connections
2. Bind the Socket to a port:
    Assign a port number to the socket so clients know where to connect
3. Listen for connections:
    Tell the socket to wait for client connections
4. Accept Connections: 
    When a client connects, the server accepts the connection and creates a new socket for with that client
5. Send and Receive Data: 
    Communicate with the client using the new socket


WHAT DO WE WANT:
1. Accepts multiple client connections
2. Communicates with clients without blocking the server    from accepting new connections.
3. Reads messages from clients and sends responses async    hronously.

BUILDING BLOCKS:
a. Session Management
   Each client connection is represented as a Session.This class:
          1. Handles communication(reading and writing)with a single client.
          2. Runs independently, allowing multiple clients to connect simulatenously.
b. Sever Core
   The server:
        1. Listens for new connection
        2. Passes each new connection to a Session.

c. Asynchronous Operations
We use async_accept, async_read_some, and async_write to:
   1. Avoid blocking the server while waiting for new clients or handling ones.
   2. Let ASIO's event loop manage readiness for read/write operations.

What is callback?
    - A callback is a function that is passed as an argument to another function , to be excuted later when an even on operation is complete. lambda function is the callback.
What is lamda function?
    - A anonymous function that can be defined inline in c++.
    - [capture list](parameters) -> return-type { body };
    - [this, self] -> this : pointer to current object
                      self: captures a specific variable, the shared pointer to the current object created from shared_from_this().

READ async_read_some, async_write, async_accept

