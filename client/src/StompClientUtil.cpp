#include "StompClientUtil.h"
#include<algorithm>

bool StompClientUtil::logIn(ConnectionHandler& connectionHandler, string host, string username, string password)
{
 	if (!connectionHandler.connect()) { 
        return false;
    }
    string frame = "CONNECT\n";
    frame += "accept-version:1.2\n";
    frame += "host:stomp.cs.bgu.ac.il\n";
    frame += "login:" + username + "\n";
    frame += "passcode:" + password + "\n";
    frame += "\n"; // End headers with a blank line
    connectionHandler.sendFrame(frame);
    string response;
    if (!connectionHandler.readFrame(response)) {
        cout << "Could not connect to server \n";
        return false;
    }
    StompFrame stompFrame(response);
    if (stompFrame.getCommand() == "ERROR") {
        string errorMessage = stompFrame.getHeader().at("message");
        cout << "Error: " << errorMessage << "\n";
        return false;
    }
    if (stompFrame.getCommand() == "CONNECTED") {
        cout << "Login successful\n";
        return true;
    }
    cout << "unknown error\n";
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

void StompClientUtil::loginLogic(ConnectionHandler& connectionHandler, string &username)
{
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
		connectionHandler.setHostPort(host, atoi(port.c_str()));
		loggedIn = StompClientUtil::logIn(connectionHandler, host, username, password);
	}
}

void StompClientUtil::epoch_to_date(int epoch, string &dateString)
{
      // Convert epoch to time_t
    std::time_t time = static_cast<std::time_t>(epoch);
    
    // Convert to tm structure for local time
    std::tm* localTime = std::localtime(&time);
    
    // Format the time into a readable string
    char buffer[100];
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime)) {
        dateString = std::string(buffer);
    }
}

void StompClientUtil::sortByDateTime(vector<Event> &v)
{
    int n = v.size();
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (v[j].get_date_time() > v[j + 1].get_date_time()){
                Event temp = v[j]; 
                v[j] = v[j+1];
                v[j+1] = temp;
            }
            else if (v[j].get_date_time() == v[j + 1].get_date_time() && v[j+1].get_name() > v[j].get_name()) {
                Event temp = v[j]; 
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
}
