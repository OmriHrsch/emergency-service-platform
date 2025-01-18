#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ConnectionHandler.h"
#include "../include/StompClientUtil.h"
#include <vector>
#include <thread>
#include <mutex>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char *argv[]) {
	bool loggedIn = false;
	ConnectionHandler* connectionHandler;
	while (!loggedIn) { //while the client is not logged in, no other commands be accepted
		string line;
		std::getline(cin, line, '\n');
		vector<string> parsedCommand = StompClientUtil::parseCommand(line);
		string command = parsedCommand[0];
		if (command != "login") {
			cout << "you should login first";
			continue;
		}
		int index = parsedCommand[1].find(':');
		string host = parsedCommand[1].substr(0, index);
		string port = parsedCommand[1].substr(index + 1);
		string username = parsedCommand[2];
		string password = parsedCommand[3];
		connectionHandler = new ConnectionHandler(host, atoi(port.c_str()));
		loggedIn = StompClientUtil::logIn(*connectionHandler, host, username, password);
	}

	//initialize two threads for client operation
	std::thread userThread ([connectionHandler](ConnectionHandler& connectionHandler) {
		while (true) {
			string line;
			std::getline(cin, line, '\n');
			vector<string> parsedCommand = StompClientUtil::parseCommand(line);
			string command = parsedCommand[0];
			if (command == "login") {
				cout << "already logged in";
			}
			else if (command == "join") {
				// Add your join logic here
			}
			else if (command == "exit") {
				// Add your exit logic here
			}
			else if (command == "report") {
				// Add your report generation logic here
			}
			else if (command == "summary") {
				// Add your summary logic here
			}
			else if (command == "logout") {
				// Add your logout logic here
			}
			else {
				// Handle unknown commands
			}
		}
	});


	std::thread socketThread ([]() {
		
	});
	return 0;
}

class ConnectionHandlerHolder {
	private:
		ConnectionHandler& handler;
	
	public:
		ConnectionHandler& getConnectionHandler() {
			return handler;
		}
		void setConnectionHandler(string host, string port) {
			ConnectionHandler ConnectionHandler(host, atoi(port.c_str()));
			handler = ConnectionHandler;
		}
};