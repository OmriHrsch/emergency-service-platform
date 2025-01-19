#include "StompClientUtil.h"

bool StompClientUtil::logIn(ConnectionHandler& connectionHandler, string host, string username, string password)
{
 	if (!connectionHandler.connect()) { 
        cout << "connection handler failed to connect, please try again";
        return false;
    }
    cout << "Connection handler connected \n";
    string frame = "CONNECT\n";
    frame += "accept-version:1.2\n";
    frame += "host:stomp.cs.bgu.ac.il\n";
    frame += "login:" + username + "\n";
    frame += "passcode:" + password + "\n";
    connectionHandler.sendLine(frame); 
    string response = "";
    if (!connectionHandler.getLine(response)) {
        cout << "Could not connect to server \n";
        return false;
    }
    cout << "server responed";
    if (response.find("User is already logged in") != std::string::npos) {
        cout << "User already logged in\n";
        return false;
    }
    if (response.find("Wrong password") != std::string::npos) {
        cout << "Wrong password\n";
        return false;
    }
    if (response.find("CONNECTED") != std::string::npos) {
        cout << "Login Succesfull";
        return true;
    }
    cout << "unknown error";
    return false;
}

vector<string> StompClientUtil::parseCommand(string command)
{
    vector<string> parseCommand = {};
    int index = 0;
    while (index < static_cast<int>(command.size()) && command[index] == ' ') {
        index++;
    }
    string nextWord = "";
    while (index < static_cast<int>(command.size())) {
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
    return parseCommand;
}

vector<string> StompClientUtil::parseAnswer(string answer) { //Parsing that would help seperating numbers from the ':' preceeding them 
    vector<string> parseAnswer = {};
    int index = 0;

    // Skip leading spaces
    while (index < static_cast<int>(answer.size()) && answer[index] == ' ') {
        index++;
    }

    string nextWord = "";
    while (index < static_cast<int>(answer.size())) {
        if (answer[index] == ' ' || answer[index] == '\n') { 
            if (!nextWord.empty()) {
                parseAnswer.push_back(nextWord);
                nextWord = "";
            }
        } else if (answer[index] == ':') { // Handle ":" specifically
            if (!nextWord.empty()) {
                parseAnswer.push_back(nextWord); 
                nextWord = ""; 
            }
        } else {
            nextWord += answer[index];
        }
        index++;
    }

    // Add the last word (or number) if not empty
    if (!nextWord.empty()) {
        parseAnswer.push_back(nextWord);
    }

    return parseAnswer;
}

void StompClientUtil::loginLogic(ConnectionHandler *connectionHandler, string &username)
{
    //delete connectionHandler;
    bool loggedIn = false;
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
    cout << "logged in successfully!";
}
