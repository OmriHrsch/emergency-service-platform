#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ConnectionHandler.h"
#include "../include/StompClientUtil.h"
#include "../include/event.h"
#include "../include/StompFrame.h"
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
	ConnectionHandler* connectionHandler = new ConnectionHandler("1.1.1.1", 1);
	StompClientUtil::loginLogic(connectionHandler, username); //this method takes care for logic operation 


	//initialize two threads for client operation
	std::thread userThread ([&connectionHandler, subscribedChannels, reciptCallbacks, eventMap, username]() mutable {
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
					event.setEventOwnerUser(username);
					eventMap[parsedReport.channel_name][username].push_back(event);
					string frame = "SEND\n";
					frame += event.toString();
					frame += "\n\0";
				}
			}
			else if (command == "summary") {
				string channelName = parsedCommand[1];
				string user = parsedCommand[2];
				string file = parsedCommand[3];
				if ((!eventMap.count(channelName)) && (!eventMap[channelName].count(user))) {
					for (Event e: eventMap[channelName][user]) {
						cout << e.toString();
					}
				}
			}
			else if (command == "logout") {
				string frame = "DISCONNECT\n";
				frame += "recipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n\0";
				reciptCallbacks.insert({reciptCounter, [connectionHandler]() mutable {
					connectionHandler->close();
					return false; 
				}});
				reciptCounter++;
			}
			else {
				cout << "Omer Atzili Ben Zona";
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
			StompFrame frame(line);
			if (frame.getCommand() == "RECEIPT"){
				int receiptID = std::stoi(frame.getHeader().at("receipt - id"));
				connected = reciptCallbacks[receiptID]();			
			}
			else if(frame.getCommand() == "MESSAGE"){
				string topic = frame.getHeader().at("destination");
				Event event(frame.getBody());
				string user = event.getEventOwnerUser();
				if (!eventMap.count(topic)) {
					eventMap.insert({topic, std::unordered_map<string, vector<Event>>()});
				}
				if(!eventMap[topic].count(user)) {
					eventMap[topic].insert({user, vector<Event>()});
				}
			}
			else if (frame.getCommand() == "ERROR"){
				connectionHandler->close();
				connected = false, error = true;
			}
		}
		if (error){ //Should threads shutdown in case of an error? if so, how do i do it? how to signal both threads to stop?
			
		}
	});
	userThread.join();
	socketThread.join();
	delete connectionHandler;
	return 0;
}

