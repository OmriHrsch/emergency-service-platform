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
	StompClientUtil::loginLogic(connectionHandler, username); //this method takes care for logic operation 
	bool loggedIn = true;
	mutex subscribedChannelsMutex;
	mutex reciptCallbacksMutex;
	mutex eventMapMutex;

	//initialize two threads for client operation
	std::thread userThread ([&connectionHandler, &subscribedChannels, &reciptCallbacks, &eventMap, &username, &loggedIn, &subscribedChannelsMutex, &reciptCallbacksMutex, &eventMapMutex]() mutable {
		int reciptCounter = 75;
		int idCounter = 0;
		while (true) {
			while (!loggedIn) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			string line;
			// std::getline(cin, line, '\n');
			getline(cin, line);
			vector<string> parsedCommand = StompClientUtil::parseCommand(line);
			string command = parsedCommand[0];
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
				names_and_events parsedReport = parseEventsFile(parsedCommand[1]);
				for (Event event: parsedReport.events) {
					event.setEventOwnerUser(username);
					string frame = "SEND\n";
					frame += "destination:/" + event.get_channel_name() + "\n\n";
					frame += event.toFrameBody();
					connectionHandler.sendFrame(frame);
				}
				cout << "reported\n";
			}
			else if (command == "summary") {
				eventMapMutex.lock();
				string channelName = parsedCommand[1];
				string user = parsedCommand[2];
				string file_path = parsedCommand[3];
				if ((eventMap.count(channelName)) && (eventMap[channelName].count(user))) {
					json summary;
					summary["Channel"] = channelName;
					int active = 0;
					int forcesArrive = 0;
					for (Event e: eventMap[channelName][user]) {
						if (e.get_general_information().at("active") == "true") {
							active++;
						}
						if (e.get_general_information().at("forces_arrival_at_scene") == "true") {
							forcesArrive++;
						}
					}
					summary["Stats"] = {
						{ "Total", std::to_string(eventMap[channelName][user].size()) },
						{ "Active", std::to_string(active) },
						{ "forces arrival at scene ", std::to_string(forcesArrive) }
					};
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
						summary["Event Reports"]["Report_" + std::to_string(i + 1)] = report;
					}
					eventMapMutex.unlock();
					std::ofstream file(file_path);
					if (file.is_open()) {
						file << summary.dump(4);
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
				reciptCallbacks.insert({reciptCounter, []() mutable {
					return false; 
				}});
				reciptCallbacksMutex.unlock();
				reciptCounter++;
			}
			else {
				cout << "unknown command\n";
			}
		}
	});

	std::thread socketThread ([&connectionHandler, &subscribedChannels,& reciptCallbacks, &eventMap, &username, &loggedIn, &subscribedChannelsMutex, &reciptCallbacksMutex, &eventMapMutex]()mutable{
		bool connected = true; //Noting if we are connected or not, and reason of disconnection (error ot logout)
		while (true){
			if (!connected) {
				connectionHandler.close();
				cout << "logged out successfully\n";
				subscribedChannelsMutex.lock();
				subscribedChannels.clear();
				subscribedChannelsMutex.unlock();
				reciptCallbacksMutex.lock();
				reciptCallbacks.clear();
				reciptCallbacksMutex.unlock();
				eventMapMutex.lock();
				eventMap.clear();
				eventMapMutex.unlock();
				StompClientUtil::loginLogic(connectionHandler, username);
				loggedIn = true;
			} 
			std::string line;
			if (!connectionHandler.readFrame(line)) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				std::exit(0);
			}
			StompFrame frame(line);
			// string frameString;
			// frame.toString(frameString);
			// cout << "recrived frame\n";
			// cout << frame.toString(frameString) + "\n";
			// cout << frame.getBody() + "\n";
			// cout << "end of frame\n";
			if (frame.getCommand() == "RECEIPT"){
				int receiptID = std::stoi(frame.getHeader().at("receipt-id"));
				reciptCallbacksMutex.lock();
				connected = reciptCallbacks[receiptID]();
				reciptCallbacksMutex.unlock();			
			}
			else if(frame.getCommand() == "MESSAGE"){
				string frameString = "";
				frame.toString(frameString);
				//cout << frameString + "\n";
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
				// cout << "topic: /" + topic + "/\n";
				// cout << "user: /" + user + "/\n";
				// cout << event.toFrameBody() + "\n";
			}
			else if (frame.getCommand() == "ERROR"){
				cout << "ERROR: ";
				cout << frame.getHeader().at("message") + "\n";
				connectionHandler.close();
				std::exit(0);
			}
		}
	});
	userThread.join();
	socketThread.join();
	return 0;
}

