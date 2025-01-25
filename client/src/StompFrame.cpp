#include "StompFrame.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>

using namespace std;

vector<string> StompFrame::str_split(string &word, char delimiter) //splitting the string by the delimiter
{
    vector<string> toReturn;
    if (word.find(delimiter) != string::npos) {
        int index = static_cast<int>(word.find(delimiter));
        toReturn.push_back(word.substr(0, index));
        toReturn.push_back(word.substr(index + 1));
    }
    return toReturn;
}

const string &StompFrame::toString(string& frameString) //converting the frame to a string
{
    frameString = command + "\n";
    for (const auto &pair : header) {
        frameString += pair.first + ":" + pair.second + "\n";
    }
    frameString += "\n";
    frameString += body;
    return frameString;
}

StompFrame::StompFrame(string &frameString) //parsing the frame
    : command(""), header(), body("") {
    stringstream ss(frameString);
    string line;
    bool inBody = false;
    string bodyContent;

    while (getline(ss, line)) {
        // Check if we are in the body
        if (line.empty()) {
            inBody = true;
            continue; // Skip empty line
        }

        if (!inBody) {
            // Parse command (first non-empty line is the command)
            if (command.empty()) {
                command = line;
            } 
            else {
                // Parse header lines
                vector<string> headerParts = str_split(line, ':');
                if (headerParts.size() == 2) {
                    string headerName = headerParts[0];
                    string headerValue = headerParts[1];
                    header[headerName] = headerValue;
                }
            }
        } 
        else {
            // Accumulate body lines
            bodyContent += line + "\n";
        }
    }

    // Remove the trailing '\n' from bodyContent if present
    if (!bodyContent.empty() && bodyContent.back() == '\n') {
        bodyContent.pop_back();
    }
    body = bodyContent;
}

const string &StompFrame::getCommand() const
{
    return command;
}

const map<string, string> &StompFrame::getHeader() const
{
    return header;
}

const string &StompFrame::getBody() const
{
    return body;
}
