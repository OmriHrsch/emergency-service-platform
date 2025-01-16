#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ConnectionHandler.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
	bool loggedIn = false;
	string line;
	while (!loggedIn) {
		std::getline(cin, line, '\n');
		int index = 0;
		string command = "";
		while (index < line.size() && line[index] != ' ') {
			command += line[index];
			index++;
		}
		index++;
		if (command != "login") {
			cout << "you should login first";
			continue;
		}
		else {
			string host = "";
			string port = "";
			string username = "";
			string password = "";

			while (index < line.size() && line[index] != ':') {
				host += line[index];
				index++;
			}
			index++;
			while (index < line.size() && line[index] != ' ') {
				port += line[index];
				index++;
			}
			index++;
			while (index < line.size() && line[index] != ' ') {
				username += line[index];
				index++;
			}
			index++;
			while (index < line.size()) {
				password += line[index];
				index++;
			}
			ConnectionHandler connectionHandler(host, atoi(port.c_str()));
			if (connectionHandler.connect()) {
				string frame = "CONNECT\n";
				frame += "accept-version:1.2\n";
				frame += "host:" + host + "\n";
				frame += "login:" + username + "\n";
				frame += "passcode:" + password + "\n";
				frame += "\n\0";
				connectionHandler.sendLine(frame);
				string response;
				if (!connectionHandler.getLine(response)) {
					cout << "Could not connect to server";
					continue;
				}
				if (response.find("already logged in") != std::string::npos) {
					cout << "User already logged in";
					continue;
				}
				if (response.find("Wrong password") != std::string::npos) {
					cout << "Wrong password";
					continue;
				}
				if (response.find("CONNECTED") == std::string::npos) {
					cout << "unknown error";
					continue;
				}
			} 				
		}
	}
	return 0;
}
