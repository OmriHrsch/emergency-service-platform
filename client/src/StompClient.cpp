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
#include <ctime>
#include "../include/json.hpp"
#include <iostream>
#include <fstream>



using json = nlohmann::json;
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
	//StompClientUtil::loginLogic(connectionHandler, username); //this method takes care for logic operation 
	bool loggedIn = false;
	mutex subscribedChannelsMutex;
	mutex reciptCallbacksMutex;
	mutex eventMapMutex;
	int reciptCounter = 0;
	int idCounter = 0;
	
	std::thread socketThread ([&connectionHandler, &subscribedChannels,& reciptCallbacks, &eventMap, &loggedIn, &subscribedChannelsMutex, &reciptCallbacksMutex, &eventMapMutex]()mutable{
		while (true){
			while (!loggedIn) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			std::string line;
			if (!connectionHandler.readFrame(line)) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				connectionHandler.close();
				loggedIn = false;
				continue;
			}
			StompFrame frame(line);
			if (frame.getCommand() == "RECEIPT"){
				int receiptID = std::stoi(frame.getHeader().at("receipt-id"));
				reciptCallbacksMutex.lock();
				loggedIn = reciptCallbacks[receiptID]();
				reciptCallbacksMutex.unlock();			
			}
			else if(frame.getCommand() == "MESSAGE"){
				string frameString = "";
				frame.toString(frameString);
				string topic = frame.getHeader().at("destination");
				if (topic.find("/") == 0) {
					topic = topic.substr(1);
				}
				Event event(frame.getBody());
				string user = event.getEventOwnerUser();
				eventMapMutex.lock();
				if (!eventMap.count(topic)) {
					eventMap.insert({topic, std::unordered_map<string, vector<Event>>()});
				}
				if(!eventMap[topic].count(user)) {
					eventMap[topic].insert({user, vector<Event>()});
				}
				eventMap[topic][user].push_back(event);
				eventMapMutex.unlock();
			}
			else if (frame.getCommand() == "ERROR"){
				cout << "ERROR: ";
				cout << frame.getHeader().at("message") + "! disconnecting...\n";
				loggedIn = false;
				connectionHandler.close();
			}
		}
		cout << "socket thread ended\n";
	});

	while (true) {
		string line;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		getline(cin, line);
		vector<string> parsedCommand = StompClientUtil::parseCommand(line);
		string command = parsedCommand[0];
		if (!loggedIn) {
			if (command == "login") {
				int index = parsedCommand[1].find(':');
				string host = parsedCommand[1].substr(0, index);
				string port = parsedCommand[1].substr(index + 1);
				username = parsedCommand[2];
				string password = parsedCommand[3];
				connectionHandler.setHostPort(host, atoi(port.c_str()));
				loggedIn = StompClientUtil::logIn(connectionHandler, host, username, password);
				if (loggedIn) {
					subscribedChannels.clear();
					reciptCallbacks.clear();
					eventMap.clear();
				}
			}
			else {
				cout << "you should login first\n";		
			}
			continue;
		}
		if (command == "login") {
			cout << "The client is already logged in, log out before trying again\n";
		}
		else if (command == "join") {
			string channelName = parsedCommand[1];
			string frame = "SUBSCRIBE\n";
			frame += "destination:" + channelName + "\n";
			frame += "id:" + std::to_string(idCounter) + "\n";
			frame += "receipt:" + std::to_string(reciptCounter) + "\n";
			frame += "\n";
			reciptCallbacksMutex.lock();
			reciptCallbacks.insert({reciptCounter, [&subscribedChannels, channelName, idCounter, &subscribedChannelsMutex]() mutable {
				subscribedChannelsMutex.lock();
				subscribedChannels[channelName] = idCounter; 
				subscribedChannelsMutex.unlock();
				cout << "joined channel " + channelName +"\n";
				return true; 
			}});
			reciptCallbacksMutex.unlock();
			reciptCounter++;
			idCounter++;
			connectionHandler.sendFrame(frame);
		}
		else if (command == "exit") {
			string channelName = parsedCommand[1];
			string frame = "UNSUBSCRIBE\n";
			subscribedChannelsMutex.lock();
			frame += "id:" + std::to_string(subscribedChannels[channelName]) + "\n";
			subscribedChannelsMutex.unlock();
			frame += "receipt:" + std::to_string(reciptCounter) + "\n";
			frame += "\n";
			reciptCallbacksMutex.lock();
			reciptCallbacks.insert({reciptCounter, [&subscribedChannels, channelName, idCounter, &subscribedChannelsMutex]() mutable { 
				subscribedChannelsMutex.lock();
				subscribedChannels.erase(channelName);
				subscribedChannelsMutex.unlock();
				cout << "Exited channel " + channelName + "\n";
				return true; 
			}});
			reciptCallbacksMutex.unlock();
			reciptCounter++;
			connectionHandler.sendFrame(frame);
		}
		else if (command == "report") {
			bool valid = true;
			names_and_events parsedReport = parseEventsFile(parsedCommand[1]);
			for (Event event: parsedReport.events) {
				event.setEventOwnerUser(username);
				string frame = "SEND\n";
				frame += "destination:/" + event.get_channel_name() + "\n\n";
				frame += event.toFrameBody();
				connectionHandler.sendFrame(frame);
				if (!subscribedChannels.count(event.get_channel_name())) {
					valid = false;
					break;
				}
			}
			if (valid) {
				cout << "reported\n";
			}
		}
		else if (command == "summary") {
			eventMapMutex.lock();
			string channelName = parsedCommand[1];
			string user = parsedCommand[2];
			string file_path = parsedCommand[3];
			if ((eventMap.count(channelName)) && (eventMap[channelName].count(user))) {
				std::stringstream summary;
				summary << "Channel: " << channelName << "\n";
				int active = 0;
				int forcesArrive = 0;
				for (Event e: eventMap[channelName][user]) {
					if (e.get_general_information().at("active") == "true") {
						active++;
					}
					if (e.get_general_information().at("forces_arrival_at_scene") == "true") {
						forcesArrive++;
					}
				};
				summary << "Stats:\n";
				summary << "Total: " << std::to_string(eventMap[channelName][user].size()) << "\n";
				summary << "Active: " << std::to_string(active) << "\n";
				summary << "forces arrival at scene: " << std::to_string(forcesArrive) << "\n\n";
				summary << "Event Reports:\n\n";
				StompClientUtil::sortByDateTime(eventMap[channelName][user]);
				for (int i = 0; i < static_cast<int>(eventMap[channelName][user].size()); i++) {
					string description = "";
					eventMap[channelName][user][i].get_description_summary(description);
					string date = "";
					eventMap[channelName][user][i].get_date_time_string(date);
					json report;
					report["city"] = eventMap[channelName][user][i].get_city();
					report["date time"] = date;
					report["event name"] = eventMap[channelName][user][i].get_name();
					report["summary"] = description;
					summary << "Report_" << std::to_string(i + 1) << "\n";
					summary << "\tcity: " << eventMap[channelName][user][i].get_city() << "\n";
					summary << "\tdate time: " << date << "\n";
					summary << "\tevent name: " << eventMap[channelName][user][i].get_name() << "\n";
					summary << "\tsummary: " << description << "\n\n";
				}
				eventMapMutex.unlock();
				std::ofstream file(file_path);
				if (file.is_open()) {
					file << summary.str(); 
					file.close();
				} else {
					std::cerr << "Error opening the file!" << std::endl;
				}
			}
			else {
				cout << "No events found for topic " + channelName + ", user " + user + "\n";
			}
		}
		else if (command == "logout") {
			string frame = "DISCONNECT\n";
			frame += "receipt:" + std::to_string(reciptCounter) + "\n";
			frame += "\n";
			connectionHandler.sendFrame(frame);
			loggedIn = false;
			reciptCallbacksMutex.lock();
			reciptCallbacks.insert({reciptCounter, [&connectionHandler]() mutable {
				cout << "logged out successfully\n";
				connectionHandler.close();
				return false; 
			}});
			reciptCallbacksMutex.unlock();
			reciptCounter++;
		}
		else {
			cout << "unknown command\n";
		}
	}	
	return 0;
}

