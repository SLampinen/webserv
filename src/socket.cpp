#include "../incl/socket.hpp"

listeningSocket::listeningSocket()
{
	
}
listeningSocket::listeningSocket(int portNum)
{
	std::cout << "making socket (with port num " << portNum << ")" << std::endl;
	this->port = portNum;
	int addrLen = sizeof(address);
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return ;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNum);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	const int enable = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return ;
	}
	if (listen(serverFd, 10) < 0)
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return ;
	}
	this->timeOfLastMsg = time(NULL);
}

listeningSocket::~listeningSocket()
{
	std::cout << "Deleting socket portNum " << port << std::endl;
}

listeningSocket::listeningSocket(const listeningSocket &var)
{
	this->serverFd = var.serverFd;
	this->address = var.address;
	this->port = var.port;
	this->newSocket = var.newSocket;
	this->timeOfLastMsg = var.timeOfLastMsg;
}

listeningSocket &listeningSocket::operator=(const listeningSocket &var)
{
	if (this != &var)
	{
		this->serverFd = var.serverFd;
		this->address = var.address;
		this->port = var.port;
		this->newSocket = var.newSocket;
		this->timeOfLastMsg = var.timeOfLastMsg;
	}
	return (*this);
}


int listeningSocket::getPortNum()
{
	return this->port;
}

struct sockaddr_in listeningSocket::getAddress()
{
	return this->address;
}


int listeningSocket::getServerFd()
{
	return this->serverFd;
}

time_t listeningSocket::getTimeOfLastMsg()
{
	return this->timeOfLastMsg;
}