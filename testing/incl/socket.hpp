#ifndef SOCKET_HPP
# define SOCKET_HPP
# include "library.hpp"
# define DEFAULT 8080

class listeningSocket
{
private:
	int serverFd;
	struct sockaddr_in address;
	int port;
	int newSocket;
public:
	listeningSocket(/* args */);
	listeningSocket(int portNum);
	~listeningSocket();
	struct sockaddr_in getAddress();
	int getServerFd();
	int getPortNum();
};


#endif