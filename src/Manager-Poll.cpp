#include "manager.hpp"

// Polling setup
void Manager::setupPollingforServers()
{
	for (size_t i = 0; i < serverList.size(); i++)
	{
		for (int j = 0; j < serverList.at(i).getNumOfPorts(); j++)
		{
			struct pollfd pfd;
			pfd.fd = serverList.at(i).listeners.at(j).getServerFd();
			pfd.events = POLLIN | POLLOUT;
			fds.push_back(pfd);
			fdsTimestamps.push_back(2147483647);
			cgiOnGoing.push_back(0);
		}
	}
}

// Poll event handling
void Manager::handlePollEvent(size_t index)
{
	if (fds[index].revents & POLLIN)
	{
		bool newConnection = acceptNewConnections(index);
		// Iterate over each server and its listeners to find the matching serverFd

		if (!newConnection)
		{
			handleClientCommunication(index);
		}
	}
	if (cgiOnGoing[index] == 1)
	{
		handleCgiWork(index);
	}
	closeInactiveConnections(index);
}

// Polling handling
void Manager::handlePolling()
{
	int pollCount = poll(fds.data(), fds.size(), POLL_TIMEOUT);
	if (pollCount < 0)
	{
		std::cerr << "Poll error: " << strerror(errno) << std::endl;
		return;
	}

	for (size_t index = 0; index < fds.size(); index++)
	{
		handlePollEvent(index);
	}
}