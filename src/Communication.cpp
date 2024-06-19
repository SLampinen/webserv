#include "../incl/manager.hpp"

// Close inactive connections
void Manager::closeInactiveConnections(size_t index)
{
	if (time(NULL) - fdsTimestamps[index] > CONNECTION_TIMEOUT)
	{
		std::cout << "Closing connection due to inactivity" << std::endl;
		close(fds[index].fd);
		fds.erase(fds.begin() + index);
		fdsTimestamps.erase(fdsTimestamps.begin() + index);
		cgiOnGoing.erase(cgiOnGoing.begin() + index);
		index--;
	}
}

// ----------- file upload --------

struct FileTransferState
{
	std::unique_ptr<std::ofstream> file;
	bool transferInProgress;

	FileTransferState() : transferInProgress(false) {}
};

std::map<int, FileTransferState> clientStates;

// Client communication
void Manager::handleClientCommunication(size_t index)
{
	char buffer[1024];
	int bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
	{
		std::cerr << "Recv error: " << strerror(errno) << std::endl;
		close(fds[index].fd);
		fds.erase(fds.begin() + index);
		fdsTimestamps.erase(fdsTimestamps.begin() + index);
		cgiOnGoing.erase(cgiOnGoing.begin() + index);
		index--; // Adjust index after erasing
		return;
	}
	else if (bytesReceived == 0)
	{
		// Connection closed by client
		std::cout << std::endl
				  << "Client closed connection" << std::endl;
		close(fds[index].fd);
		fds.erase(fds.begin() + index);
		fdsTimestamps.erase(fdsTimestamps.begin() + index);
		cgiOnGoing.erase(cgiOnGoing.begin() + index);
		index--; // Adjust index after erasing
		return;
	}
	else
	{
		// if request too large
		std::string receivedData(buffer, bytesReceived);
		bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		while (bytesReceived > 0)
		{
			std::string rest(buffer, bytesReceived);
			receivedData.append(rest);
			bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		}

		// If cgi is ongoing, do not handle the request
		fdsTimestamps[index] = time(NULL);
		cgiOnGoing[index] = 0;
		for (int k = 0; k < pids.size(); k++)
		{
			if (index == pids.at(k).second)
			{
				kill(pids.at(k).first, 9);
				pids.erase(pids.begin() + k);
			}
		}

		if (!clientStates[fds[index].fd].transferInProgress)
		{
			printf("--inside find GET etc.--\n");
			if (receivedData.find("GET") != std::string::npos)
			{
				std::cout << "GETTING" << std::endl;
				handleGet(receivedData, fds, index);
			}
			else if (receivedData.find("POST") != std::string::npos)
			{
				std::cout << "POSTING" << std::endl;
				handlePost(receivedData, fds, index);
			}
			else if (receivedData.find("DELETE") != std::string::npos)
			{
				std::cout << "DELETING" << std::endl;
				handleDelete(receivedData, fds, index);
			}
			else
			{
				std::cout << "OTHER METHOD" << std::endl;
				handleOther(receivedData, fds, index);
			}
			if (!isLastChunk(receivedData))
				clientStates[fds[index].fd].transferInProgress = true;
		}
		else
		{
			// continue receiving file
			printf("--inside continue receiving file--\n");
			while (bytesReceived > 0 && !isLastChunk(receivedData))
			{
				*(clientStates[fds[index].fd].file) << receivedData;
				bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
				receivedData.append(buffer, bytesReceived);
			}

			if (isLastChunk(receivedData))
			{
				// File transfer complete
				clientStates[fds[index].fd].transferInProgress = false;
			}
		}
		// start timer for timeout
	}
}

bool Manager::isLastChunk(const std::string &data)
{
	printf("--inside isLastChunk--\n");
	// This is a simplified check and might not work with all HTTP servers and clients.
	// A more robust implementation would parse the chunk sizes and the HTTP headers.
	return data.find("\r\n0\r\n\r\n") != std::string::npos;
}
