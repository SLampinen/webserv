#include "manager.hpp"

Manager::Manager() : default_responses("DefaultResponses") {}

Manager::~Manager() {}

void setNonBlocking(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

// New connection
bool Manager::acceptNewConnections(size_t index)
{
	for (size_t j = 0; j < serverList.size(); j++)
	{
		for (int k = 0; k < serverList.at(j).getNumOfPorts(); k++)
		{
			if (fds[index].fd == serverList.at(j).listeners.at(k).getServerFd())
			{
				while (true)
				{
					struct sockaddr_in clientAddr;
					socklen_t clientAddrLen = sizeof(clientAddr);
					int clientFd = accept(fds[index].fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
					if (clientFd < 0)
					{
						if (errno == EWOULDBLOCK || errno == EAGAIN)
						{
							// No more clients to accept
							break;
						}
						else
						{
							break;
						}
					}
					setNonBlocking(clientFd);
					// Add the new client socket to the poll list
					struct pollfd clientPfd;
					clientPfd.fd = clientFd;
					clientPfd.events = POLLIN | POLLOUT;
					fds.push_back(clientPfd);
					fdsTimestamps.push_back(time(NULL));
					cgiOnGoing.push_back(0);
					serverIndex.push_back(std::make_pair(clientFd, j));
				}
				return true; // Return true as soon as a new connection is accepted
			}
		}
	}
	return false; // Return false if no new connection was accepted
}

// Run
void Manager::run(std::string configFile)
{
	ConfigParser config_parse(configFile);
	readConfig(config_parse);
	setupPollingforServers();
	while (true)
	{
		handlePolling();
	}
}
