#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"
#include "ConnectionHandler.h"

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

ConnectionHandler::ConnectionHandler(string host, short port) : host_(host), port_(port), io_service_(),
                                                                socket_(io_service_) {}

ConnectionHandler::~ConnectionHandler() {
	if (socket_.is_open())
		close();
}

bool ConnectionHandler::connect() {
	std::cout << "Starting connect to "
	          << host_ << ":" << port_ << std::endl;
	try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
	}
	catch (std::exception &e) {
		std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
	size_t tmp = 0;
	boost::system::error_code error;
	if (error == boost::asio::error::eof) {
		std::cerr << "Connection closed by the server." << std::endl;
		return false;
	}
	if (!socket_.is_open()) {
		std::cerr << "Error: Socket is not open." << std::endl;
		return false;
	}
	

	try {
		while (!error && bytesToRead > tmp) {
			tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);;
		}
		if (error)
			throw boost::system::system_error(error);
	} catch (std::exception &e) {
		std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		cout << "getBytes failed" << endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
	int tmp = 0;
	boost::system::error_code error;
	try {
		while (!error && bytesToWrite > tmp) {
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
		}
		if (error)
			throw boost::system::system_error(error);
	} catch (std::exception &e) {
		std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::getLine(std::string &line) {
	return getFrameAscii(line, '\n');
}

bool ConnectionHandler::sendLine(std::string &line) {
	return sendFrameAscii(line, '\n');
}


bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter) {
	char ch;
	// Stop when we encounter the null character.
	// Notice that the null character is not appended to the frame string.
	try {
		do {
			if (!getBytes(&ch, 1)) {
				cout << frame << endl;
				return false;
			}
			if (ch != '\0')
				frame.append(1, ch);
		} while (delimiter != ch);
	} catch (std::exception &e) {
		std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}
//login 127.0.0.1:2020 tom tom
bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter) {
	bool result = sendBytes(frame.c_str(), frame.length());
	if (!result) return false;
	return sendBytes(&delimiter, 1);
}

// Close down the connection properly.
void ConnectionHandler::close() {
	try {
		if (socket_.is_open())
			socket_.close();
	} catch (...) {
		std::cout << "closing failed: connection already closed" << std::endl;
	}
}
bool ConnectionHandler::isSocketOpen()
{
    return socket_.is_open();
}

void ConnectionHandler::setHostPort(std::string host, short port)
{
	this->host_ = host;
	this->port_ = port;
}

bool ConnectionHandler::sendFrame(std::string &line)
{
    if (!sendLine(line)) {return false;}
	return sendBytes({"\0"}, 1);
}

bool ConnectionHandler::readFrame(std::string &line){
	string holder = "";
	int hasBody = 0;
	if(!getLine(holder)){return false;}
	line = line + holder;
	if (holder == "MESSAGE\n") {;
		while (holder.find("description") != 0) {
			holder.clear();
			if(!getLine(holder)){return false;}
			line = line + holder;
		}
		holder.clear();
		if(!getLine(holder)){return false;}
		line = line + holder;
	}
	else {
		while (hasBody >= 0) {
			holder.clear();
			if(!getLine(holder)){return false;}
			line = line + holder;
			if (holder == "\n") {
				hasBody--;
			}
		}
	}
	return true;
}
