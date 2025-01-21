#include "../include/event.h"
#include "../include/json.hpp"
#include "../include/StompFrame.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstring>
#include "event.h"



using namespace std;
using json = nlohmann::json;

Event::Event(std::string channel_name, std::string city, std::string name, int date_time,
             std::string description, std::map<std::string, std::string> general_information)
    : channel_name(channel_name), city(city), name(name),
      date_time(date_time), description(description), general_information(general_information), eventOwnerUser("")
{
}

Event::~Event()
{
}

void Event::setEventOwnerUser(std::string setEventOwnerUser) {
    eventOwnerUser = setEventOwnerUser;
}

const std::string &Event::getEventOwnerUser() const {
    return eventOwnerUser;
}

const std::string &Event::get_channel_name() const
{
    return this->channel_name;
}

const std::string &Event::get_city() const
{
    return this->city;
}

const std::string &Event::get_name() const
{
    return this->name;
}

const void Event::get_description_summary(std::string& str) const
{
    if (static_cast<int>(description.size()) > 27) {
        str = description.substr(0, 27) + "...";
    } else {
        str = description;
    }
}

const void Event::get_date_time_string(std::string& str) const
{
    string dateString = "";
    epoch_to_date(date_time, dateString);
    str = dateString;
}

int Event::get_date_time() const
{
    return this->date_time;
}

const std::map<std::string, std::string> &Event::get_general_information() const
{
    return this->general_information;
}

std::string Event::toString() const
{
    string dateString = "";
    epoch_to_date(date_time, dateString);
    std::ostringstream oss;

    // Add city
    oss << "city:" << city << "\n";

    // Add date time
    oss << "date time:" << dateString << "\n";

    // Add event name
    oss << "event name:" << name << "\n";

    // Add description
    if (static_cast<int>(description.size()) > 27) {
        oss << "summary: " << description.substr(0, 27) << "...\n";
    }
    else {
        oss << "summary: " << description + "\n";
    }
    

    return oss.str();
}

std::string Event::toFrameBody() const
{
    std::ostringstream oss;

    // Add user
    oss << "user:" << eventOwnerUser << "\n";

    // Add city
    oss << "city:" << city << "\n";

    // Add event name
    oss << "event name:" << name << "\n";

    // Add date time
    oss << "date time:" << date_time << "\n";

    // Add general information
    oss << "general information:\n";
    for (const auto &pair : general_information) {
        // if (pair.first == "forces_arrival_at_scene") {
        //     oss << " " << "forces arrival at scene: " << pair.second << "\n";
        // }
        // else {
            oss << "\t" << pair.first << ":" << pair.second << "\n";
        //}
    }

    // Add description
    oss << "description:\n" << description;

    return oss.str();
}

void Event::epoch_to_date(int epoch, std::string &dateString)
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

const std::string &Event::get_description() const
{
    return this->description;
}

Event::Event(const std::string &frame_body): channel_name(""), city(""), 
                                             name(""), date_time(0), description(""), general_information(),
                                             eventOwnerUser("")
{
    stringstream ss(frame_body);
    string line;
    string eventDescription;
    map<string, string> general_information_from_string;
    bool inGeneralInformation = false;
    while(getline(ss,line,'\n')){
        if(line.find(':') != string::npos) {
            vector<string> lineArgs = StompFrame::str_split(line, ':');
            string key = lineArgs.at(0);
            string val;
            if(lineArgs.size() == 2) {
                val = lineArgs.at(1);
            }
            if(key == "user") {
                eventOwnerUser = val;
            }
            if(key == "channel name") {
                channel_name = val;
            }
            if(key == "city") {
                city = val;
            }
            else if(key == "event name") {
                name = val;
            }
            else if(key == "date time") {
                date_time = std::stoi(val);
            }
            else if(key == "general information") {
                inGeneralInformation = true;
                continue;
            }
            else if(key == "description") {
                while(getline(ss,line,'\n')) {
                    eventDescription += line + "\n";
                }
                description = eventDescription;
            }

            if(inGeneralInformation) {
                general_information_from_string[key.substr(1)] = val;
            }
        }
    }
    general_information = general_information_from_string;
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string channel_name = data["channel_name"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event_name"];
        std::string city = event["city"];
        int date_time = event["date_time"];
        std::string description = event["description"];
        std::map<std::string, std::string> general_information;
        for (auto &update : event["general_information"].items())
        {
            if (update.value().is_string())
                general_information[update.key()] = update.value();
            else
                general_information[update.key()] = update.value().dump();
        }

        events.push_back(Event(channel_name, city, name, date_time, description, general_information));
    }
    names_and_events events_and_names{channel_name, events};

    return events_and_names;
}