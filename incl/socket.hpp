#ifndef SOCKET_HPP
# define SOCKET_HPP
# include "library.hpp"
# define DEFAULTPORT 8080

class listeningSocket
{
private:
	int serverFd;
	struct sockaddr_in address;
	int port;
	int newSocket;
	time_t timeOfLastMsg;
public:
	listeningSocket();
	listeningSocket(int portNum);
	~listeningSocket();
	listeningSocket(const listeningSocket &var);
	listeningSocket& operator=(const listeningSocket &var);
	struct sockaddr_in getAddress();
	int getServerFd();
	int getPortNum();
	time_t getTimeOfLastMsg();
};


#endif