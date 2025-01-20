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
using std::mutex;


int main(int argc, char *argv[]) {
	string username;
	std::unordered_map<string, int> subscribedChannels;
	std::unordered_map<int, std::function<bool()>> reciptCallbacks;
	std::unordered_map<string, std::unordered_map<string, vector<Event>>> eventMap;
	ConnectionHandler connectionHandler("host", 1111);
	StompClientUtil::loginLogic(connectionHandler, username); //this method takes care for logic operation 
	bool loggedIn = true;

	if (!connectionHandler.isSocketOpen()) {
		cout << "Socket closed after\n";
		return 0;
	}

	//initialize two threads for client operation
	std::thread userThread ([&connectionHandler, &subscribedChannels, &reciptCallbacks, &eventMap, &username, &loggedIn]() mutable {
		int reciptCounter = 1;
		int idCounter = 1;
		while (true) {
			while (!loggedIn) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			string line;
			std::getline(cin, line, '\n');
			vector<string> parsedCommand = StompClientUtil::parseCommand(line);
			string command = parsedCommand[0];
			if (command == "login") {
				cout << "The client is already logged in, log out before trying again\n";
			}
			else if (command == "join") {
				string channelName = parsedCommand[1];
				string frame = "SUBSCRIBE\n";
				frame += "destination:/" + channelName + "\n";
				frame += "id:" + std::to_string(idCounter) + "\n";
				frame += "receipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n";
				reciptCallbacks.insert({reciptCounter, [&subscribedChannels, channelName, idCounter]() mutable {
					 subscribedChannels[channelName] = idCounter; 
					 cout << "joined to " + channelName +"\n";
					 return true; 
				}});
				reciptCounter++;
				idCounter++;
				connectionHandler.sendFrame(frame);
			}
			else if (command == "exit") {
				string channelName = parsedCommand[1];
				string frame = "UNSUBSCRIBE\n";
				frame += "id:" + std::to_string(subscribedChannels[channelName]) + "\n";
				frame += "receipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n";
				reciptCallbacks.insert({reciptCounter, [subscribedChannels, channelName, idCounter]() mutable { 
					subscribedChannels.erase(channelName);
					cout << "unsubscribed" + channelName + "successfully\n";
					return true; 
				}});
				reciptCounter++;
				connectionHandler.sendFrame(frame);
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
					frame += "\n";
					connectionHandler.sendFrame(frame);
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
				frame += "receipt:" + std::to_string(reciptCounter) + "\n";
				frame += "\n";
				connectionHandler.sendFrame(frame);
				loggedIn = false;
				reciptCallbacks.insert({reciptCounter, [&subscribedChannels, &reciptCallbacks, &eventMap, &username, &loggedIn]() mutable {
					subscribedChannels.clear();
					reciptCallbacks.clear();
					eventMap.clear();
					return false; 
				}});
				reciptCounter++;
			}
			else {
				cout << "Omer Atzili Ben Zona\n";
			}
		}
	});

	std::thread socketThread ([&connectionHandler, &subscribedChannels,& reciptCallbacks, &eventMap, &username, &loggedIn]()mutable{
		bool connected = true, error = false; //Noting if we are connected or not, and reason of disconnection (error ot logout)
		while (true){
			if (!connected) {
				connectionHandler.close();
				cout << "logged out successfully\n";
				StompClientUtil::loginLogic(connectionHandler, username);
				loggedIn = true;
			} 
			std::string line;
			if (!connectionHandler.readFrame(line)) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				error = true;
				break;
			}
			StompFrame frame(line);
			if (frame.getCommand() == "RECEIPT"){
				int receiptID = std::stoi(frame.getHeader().at("receipt-id"));
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
				connectionHandler.close();
				connected = false, error = true;
			}
		}
		connectionHandler.close(); //Closing the client's connection after user disconnects, after normal logout or error
		if (error){ //Is there a specific error shutdown logic?
			
		}
	});
	userThread.join();
	socketThread.join();
	return 0;
}

