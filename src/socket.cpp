#include "socket.hpp"

listeningSocket::listeningSocket() {}

listeningSocket::listeningSocket(int portNum)
{
	this->port = portNum;
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cerr << "Port " << portNum << ": ERROR, " << strerror(errno) << std::endl;
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
		std::cerr << "Port " << portNum << ": ERROR, " << strerror(errno) << std::endl;
		return ;
	}
	if (listen(serverFd, 10) < 0)
	{
		std::cerr << "Port " << portNum << ": ERROR, " << strerror(errno) << std::endl;
		return ;
	}
}

listeningSocket::~listeningSocket() {}

listeningSocket::listeningSocket(const listeningSocket &var)
{
	this->serverFd = var.serverFd;
	this->address = var.address;
	this->port = var.port;
	this->newSocket = var.newSocket;
}

listeningSocket &listeningSocket::operator=(const listeningSocket &var)
{
	if (this != &var)
	{
		this->serverFd = var.serverFd;
		this->address = var.address;
		this->port = var.port;
		this->newSocket = var.newSocket;
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
