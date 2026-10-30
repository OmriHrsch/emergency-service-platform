# Emergency Service Platform

A client-server platform for reporting and following emergency events in real
time. The project uses the STOMP 1.2 protocol, a Java server, and a
multithreaded C++ client.

This is a two-person academic project by Omri Hirsch and Tom Mimran. The public
repository keeps the original Git history and authorship.

## What it does

- Registers users and manages authenticated sessions.
- Subscribes users to emergency channels.
- Publishes emergency events from JSON files.
- Delivers channel updates to connected users.
- Creates ordered event summaries in text files.
- Runs the server with a thread-per-client or Reactor model.

## Architecture

| Component | Technology | Responsibility |
| --- | --- | --- |
| Server | Java, Maven | STOMP connections, users, subscriptions, and message delivery |
| Client | C++11, Boost.Asio | Commands, concurrent network input, reports, and summaries |
| Protocol | STOMP 1.2 | Text frames between the client and the server |

## Requirements

- JDK 8 or newer
- Maven 3.6 or newer
- A C++11 compiler
- Make
- Boost.Asio and Boost.System

Install the dependencies on macOS with Homebrew:

```bash
brew install boost maven openjdk
```

Install the dependencies on Ubuntu:

```bash
sudo apt-get install build-essential libboost-all-dev maven openjdk-17-jdk
```

You can also open the repository in a development container. The container
includes the required build tools.

## Run the platform

Start the server in thread-per-client mode:

```bash
cd server
mvn clean compile
mvn exec:java -Dexec.mainClass=bgu.spl.net.impl.stomp.StompServer \
  -Dexec.args="7777 tpc"
```

Use `reactor` instead of `tpc` to run the Reactor server:

```bash
mvn exec:java -Dexec.mainClass=bgu.spl.net.impl.stomp.StompServer \
  -Dexec.args="7777 reactor"
```

In a second terminal, build and start the client:

```bash
cd client
make
./bin/StompEMIClient
```

Enter these commands in the client:

```text
login 127.0.0.1:7777 omri password
join police
report data/events1.json
summary police omri summary.txt
exit police
logout
```

The server registers a new username during the first login. Later logins must
use the same password.

## Client commands

| Command | Purpose |
| --- | --- |
| `login {host:port} {username} {password}` | Connect and authenticate |
| `join {channel}` | Subscribe to a channel |
| `exit {channel}` | Unsubscribe from a channel |
| `report {json-file}` | Publish the events in a JSON file |
| `summary {channel} {user} {output-file}` | Write an event summary |
| `logout` | End the session |

## Project structure

```text
client/
  data/       Sample emergency event files
  include/    C++ headers
  src/        C++ client source
server/
  src/        Java server source
```

## Contributors

- Omri Hirsch
- Tom Mimran

This repository is an academic prototype. It is not a production emergency
service.
