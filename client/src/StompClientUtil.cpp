#include "StompClientUtil.h"

bool StompClientUtil::logIn(ConnectionHandler& connectionHandler, string host, string username, string password)
{
 	if (!connectionHandler.connect()) { 
        cout << "connection handler failed to connect, please try again";
        return false;
    }
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
        return false;
    }
    if (response.find("already logged in") != std::string::npos) {
        cout << "User already logged in";
        return false;
    }
    if (response.find("Wrong password") != std::string::npos) {
        cout << "Wrong password";
        return false;
    }
    if (response.find("CONNECTED") == std::string::npos) {
        cout << "Connected Succesfully";
        return true;
    }
    cout << "unknown error";
    return false;
}

vector<string> StompClientUtil::parseCommand(string command)
{
    vector<string> parseCommand = {};
    int index = 0;
    while (index < command.size() && command[index] == ' ') {
        index++;
    }
    string nextWord = "";
    while (index < command.size()) {
        if (command[index] == ' ') {
            parseCommand.push_back(nextWord);
            nextWord = "";
        }
        else {
            nextWord += command[index];
        }
        index++;
    }
    parseCommand.push_back(nextWord);
    return vector<string>();
}
