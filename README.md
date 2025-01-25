# SPL Assignment 3: Emergency Service Platform

This project is part of the SPL course and implements an "Emergency Service" platform. It enables users to subscribe to emergency channels, report emergencies, and receive updates in real time using the STOMP protocol. The platform consists of a server (Java) and client (C++) implementation.

## Features

### Server
- Implements a STOMP-based messaging server.
- Supports two modes:
  - *Thread-Per-Client (TPC)*: Each client is handled by a dedicated thread.
  - *Reactor*: Event-driven architecture for handling multiple clients efficiently.
- Handles connections, subscriptions, and message distribution to relevant topics.

### Client
- Enables users to:
  - Subscribe and unsubscribe to emergency channels.
  - Report emergencies from JSON files.
  - View summaries of emergency events.
  - Manage user sessions with login and logout functionality.
- Implements multithreading for handling user inputs and server responses concurrently.

---

## Technologies Used

- *Java*: Server-side implementation.
- *C++*: Client-side implementation.
- *STOMP (Simple-Text-Oriented Messaging Protocol)*: For communication between server and clients.
- *Maven*: Build and dependency management tool for the server.
- *Makefile*: Build tool for the client.

---

## Setup and Usage

### Prerequisites
- *Java Development Kit (JDK)*
- *C++ Compiler*
- *Maven*
- *Make* utility
- Compatible *Linux environment* (Docker provided for lab usage).

### Build and Run Instructions

#### Server
1. Navigate to the server directory.
2. Compile the server:
   bash
   mvn compile
   
3. Run the server:
   bash
   mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="<port> <reactor|tpc>"
   
   Replace <port> with the desired port number and <reactor|tpc> with the server mode.

#### Client
1. Navigate to the client directory.
2. Build the client:
   bash
   make
   
3. Run the client:
   bash
   ./bin/StompEMIClient
   

---

## Commands

### Client Commands
- *Login*: login {host:port} {username} {password}
- *Join Channel*: join {channel_name}
- *Exit Channel*: exit {channel_name}
- *Report Events*: report {file}
- *Summarize Events*: summary {channel_name} {user} {file}
- *Logout*: logout

Refer to the [Assignment Instructions](./SPL251__Assignment_3_instructions.pdf) for detailed examples and explanations.

---

## File Structure
- *server/*: Contains Java server code and Maven configuration.
- *client/*: Contains C++ client code, Makefile, and binaries.
- *data/*: Sample emergency event JSON files.
- *instructions/*: Assignment instructions and detailed implementation requirements.

---

## Contributors
- Omri Hirsch
- Tom Mimran
