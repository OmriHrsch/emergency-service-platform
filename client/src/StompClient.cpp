#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ConnectionHandler.h"
#include "../include/StompClientUtil.h"
#include "../include/event.h"
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <functional>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;


int main(int argc, char *argv[]) {
	std::map<int, string> subsribedChannels();
	std::unordered_map<int, std::function<void>()> reciptCallbacks();
	std::unordered_map<string, std::unordered_map<string, event>>;
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
	std::thread userThread ([&connectionHandler](ConnectionHandler& connectionHandler) {
		int reciptCounter = 1;
		int idCounter = 1;
		while (true) {
			string line;
			std::getline(cin, line, '\n');
			vector<string> parsedCommand = StompClientUtil::parseCommand(line);
			string command = parsedCommand[0];
			if (command == "login") {
				cout << "The client is already logged in, log out before trying again";
			}
			else if (command == "join") {
				string channelName = parsedCommand[1];
				string frame = "SUBSCRIBE\n";
				frame += "destination:/" + channelName + "\n";
				frame += "id:" + std::to_string(id) + "\n";
				frame += "recipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n\0";
				connectionHandler.sendLine(frame);
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

	delete connectionHandler;
	return 0;
}
