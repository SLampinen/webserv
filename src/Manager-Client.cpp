#include "manager.hpp"

void Manager::closeConnections(size_t index, std::string message)
{
	std::cout << message << std::endl;
	close(fds[index].fd);
	fds.erase(fds.begin() + index);
	fdsTimestamps.erase(fdsTimestamps.begin() + index);
	cgiOnGoing.erase(cgiOnGoing.begin() + index);
	index--;
}

// Close inactive connections
void Manager::closeInactiveConnections(size_t index)
{
	if (time(NULL) - fdsTimestamps[index] > CONNECTION_TIMEOUT)
		closeConnections(index, "Connection closed due to inactivity");
}

void Manager::handleClientCommunication(size_t index)
{
	char buffer[1024];
	int bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
	if (bytesReceived == -1)
	{
		closeConnections(index, "Recv error: " + std::string(strerror(errno)));
		return;
	}
	else if (bytesReceived == 0)
	{
		closeConnections(index, "Connection closed by client");
		return;
	}
	else
	{
		std::string receivedData(buffer, bytesReceived);
		std::cout << "Received data before while: " << receivedData << std::endl;
		bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		while (bytesReceived > 0)
		{
			std::string rest(buffer, bytesReceived);
			receivedData.append(rest);
			bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		}
		std::cout << "Received data after while: " << receivedData << std::endl;
		std::cout << "Received data size: " << receivedData.size() << std::endl;

		if (receivedData.find("Content-Length:") != std::string::npos)
		{

			std::string contentLength = receivedData.substr(receivedData.find("Content-Length:") + 16);
			contentLength = contentLength.substr(0, contentLength.find("\r\n"));
			int length = std::stoi(contentLength);
			bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);

			size_t headerEndPos = receivedData.find("\r\n\r\n");
			if (headerEndPos != std::string::npos)
			{
				// The header size is the position of the end of the header plus 4 (the length of "\r\n\r\n")
				size_t headerSize = headerEndPos + 4;
				// Subtract the header size from the length
				length += headerSize;

			}

			while (bytesReceived < length)
			{
				if (bytesReceived == -1)
				{
					closeConnections(index, "Recv error: " + std::string(strerror(errno)));
					break;
				}
				else if (bytesReceived == 0)
				{
					closeConnections(index, "Connection closed by client");
					break;
				}
				std::string rest(buffer, bytesReceived);
				receivedData.append(rest);
				length -= bytesReceived;
				bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
																		printf("HI MOTHERFUCKER\n");
			
			}
		}
		if (receivedData.find("Expect:") != std::string::npos)
		{
			std::string response = "HTTP/1.1 413 Request Entity Too Large\r\n\r\n";
			send(fds[index].fd, response.c_str(), response.size(), 0);
			std::cout << "File too big" << std::endl;
			close(fds[index].fd);
			fds.erase(fds.begin() + index);
			fdsTimestamps.erase(fdsTimestamps.begin() + index);
			cgiOnGoing.erase(cgiOnGoing.begin() + index);
			index--;
			return;
		}

		// If cgi is ongoing, throw out previous request, start new one if necessary
		fdsTimestamps[index] = time(NULL);
		cgiOnGoing[index] = 0;
		for (size_t k = 0; k < pids.size(); k++)
		{
			if (index == pids.at(k).second)
			{
				kill(pids.at(k).first, 9);
				// gets rid of the temp file that child made
				std::string name = serverList.at(serverIndex.at(k).second).getRootDir();
				name.append("temp");
				name.append(std::to_string(index));
				unlink(name.c_str());
				pids.erase(pids.begin() + k);
			}
		}
		// if (!clientStates[fds[index].fd].transferInProgress)
		// {
		// clientStates[fds[index].fd].transferInProgress = true;
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
		else if ((receivedData.find("HEAD") != std::string::npos || receivedData.find("PUT") != std::string::npos ||
				  receivedData.find("CONNECT") != std::string::npos || receivedData.find("OPTIONS") != std::string::npos ||
				  receivedData.find("TRACE") != std::string::npos || receivedData.find("PATCH") != std::string::npos) &&
				 receivedData.find("Content-Type:") == std::string::npos)
		{
			std::cout << "OTHER METHOD" << std::endl;
			handleOther(receivedData, fds, index);
		}
		else
		{

			std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
			send(fds[index].fd, response.c_str(), response.size(), 0);
			std::cout << "Bad Request" << std::endl;
			close(fds[index].fd);
			fds.erase(fds.begin() + index);
			fdsTimestamps.erase(fdsTimestamps.begin() + index);
			cgiOnGoing.erase(cgiOnGoing.begin() + index);
			index--;
			return;
		}
		// }
		// else
		// {
		// 	handleContinue(index);
		// 	// 	// check if file is complete, then set tranferinprogress to false.
		// 	std::cerr << "time now : " << time(NULL) << " and timestamp : " << fdsTimestamps.at(index) << std::endl;
		// 	if (fdsTimestamps[index] - time(NULL) > REQUEST_TIMEOUT)
		// 	{
		// 		handleTimeout(index);
		// 	}
		// }
	}
}