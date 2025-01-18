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
	string username;
	std::unordered_map<string, int> subscribedChannels;
	std::unordered_map<int, std::function<bool()>> reciptCallbacks;
	std::unordered_map<string, std::unordered_map<string, vector<Event>>> eventMap;
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
		username = parsedCommand[2];
		string password = parsedCommand[3];
		connectionHandler = new ConnectionHandler(host, atoi(port.c_str()));
		loggedIn = StompClientUtil::logIn(*connectionHandler, host, username, password);
	}

	//initialize two threads for client operation
	std::thread userThread ([connectionHandler, subscribedChannels, reciptCallbacks, eventMap, username]() mutable {
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
				frame += "id:" + std::to_string(idCounter) + "\n";
				frame += "recipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n\0";
				reciptCallbacks.insert({reciptCounter, [subscribedChannels, channelName, idCounter]() mutable { subscribedChannels[channelName] = idCounter; return true; }});
				reciptCounter++;
				idCounter++;
				connectionHandler->sendLine(frame);
			}
			else if (command == "exit") {
				string channelName = parsedCommand[1];
				string frame = "UNSUBSCRIBE\n";
				frame += "id:" + std::to_string(subscribedChannels[channelName]) + "\n";
				frame += "recipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n\0";
				reciptCallbacks.insert({reciptCounter, [subscribedChannels, channelName, idCounter]() mutable { subscribedChannels.erase(channelName); return true; }});
				reciptCounter++;
				connectionHandler->sendLine(frame);
			}
			else if (command == "report") {
				names_and_events parsedReport = parseEventsFile(parsedCommand[1]);
				if (!eventMap.count(parsedReport.channel_name)) {
					eventMap.insert({parsedReport.channel_name, std::unordered_map<string, vector<Event>>()});
				}
				if(!eventMap[parsedReport.channel_name].count(username)) {
					eventMap[parsedReport.channel_name].insert({username, vector<Event>()});
				}
				for (Event event: parsedReport.events) {
					eventMap[parsedReport.channel_name][username].push_back(event);
					string frame = "SEND\n";
					frame += "destination:/" + event.get_channel_name() + "\n\n";
					frame += "user:" + username + "\n";
					frame += "city:" + event.get_city() + "\n";
					
				}
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


	std::thread socketThread ([connectionHandler,subscribedChannels,reciptCallbacks,eventMap]()mutable{
		//To implement: as loop, that checks as long as we shouldnt terminate/ didnt get an error we should stay connected
		bool connected = true, error = false; //Noting if we are connected or not, and reason of diesconnection (error ot logout)
		while (connected){ 
			std::string line;
			if (!connectionHandler->getLine(line)) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				error = true;
				break;
			}
			vector<string> parseAnswer = StompClientUtil::parseAnswer(line);
			string frameType = parseAnswer[0];
			int receiptID = std::stoi(parseAnswer[4]);
			if (frameType == "RECEIPT"){
				connected = reciptCallbacks[receiptID]();			
			}
			else if(frameType == "MESSAGE"){
				
			}
			else if (frameType == "ERROR"){
				connectionHandler->close();
				connected = false, error = true;
			}
		}
		if (error){ //Should threads shutdown in case of an error? if so, how do i do it? how to signal both threads to stop?
			
		}
	});

	delete connectionHandler;
	return 0;
}
