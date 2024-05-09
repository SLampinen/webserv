#include "../incl/socket.hpp"

listeningSocket::listeningSocket()
{
	this->port = DEFAULT;
	int addrLen = sizeof(address);
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cout << "ERROR, socket failed" << std::endl;
		return ;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(DEFAULT);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	const int enable = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		std::cout << "ERROR, socket failed" << std::endl;
		return ;
	}
	if (listen(serverFd, 10) < 0)
	{
		std::cout << "ERROR, listen failed" << std::endl;
		return ;
	}

}

listeningSocket::listeningSocket(int portNum)
{
	this->port = portNum;
	int addrLen = sizeof(address);
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cout << "ERROR, socket failed" << std::endl;
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
		std::cout << "ERROR, socket failed" << std::endl;
		return ;
	}
	if (listen(serverFd, 10) < 0)
	{
		std::cout << "ERROR, listen failed" << std::endl;
		return ;
	}

}

listeningSocket::~listeningSocket()
{
	std::cout << "Deleting socket" << std::endl;
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
