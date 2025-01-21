#pragma once

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"
#include "../include/event.h"
#include <vector>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

class StompClientUtil {
    
    public:
        static bool logIn(ConnectionHandler& connectionHandler, string host, string username, string password);
        static vector<string> parseCommand(string command);
        static vector<string> parseAnswer(string answer);
        static void loginLogic(ConnectionHandler& connectionHandler, string& username);
        static void epoch_to_date(int epoch, string& dateString);
        static void sortByDateTime(vector<Event>& v);
};
