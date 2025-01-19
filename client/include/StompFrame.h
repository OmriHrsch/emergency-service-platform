#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector> 
#include <sstream>

using std::string;
using std::map;
using std::vector;

class StompFrame {

    private:
        string command;
        map<string, string> header;
        string body;
   
    public:
        StompFrame(string& frameString);
        const string& getCommand() const;
        const map<string, string>& getHeader() const;
        const string& getBody() const; 
        static vector<string> str_split(string& word, char delimiter);
};