#include "../incl/manager.hpp"

void Manager::closeConnections(size_t index)
{
	std::cout << "Closing connection due to inactivity" << std::endl;
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
		closeConnections(index);
}

void Manager::handleClientCommunication(size_t index)
{
	char buffer[1024];
	std::cout << "Handling client communication" << std::endl;
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
		std::cout << "after first recv. bytesreceived:" << bytesReceived << std::endl;
		if (receivedData.find("Content-Length:") != std::string::npos && bytesReceived != -1)
		{
			std::string contentLength = receivedData.substr(receivedData.find("Content-Length:") + 16);
			contentLength = contentLength.substr(0, contentLength.find("\r\n"));
			size_t length = std::stoi(contentLength);
			bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
			while (receivedData.size() < length)
			{
				std::string rest(buffer, bytesReceived);
				receivedData.append(rest);
				length -= bytesReceived;
				bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
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

		std::cout << "Received data FIRST: " << receivedData << std::endl;
		// If cgi is ongoing, throw out previous request, start new one if necessary
		fdsTimestamps[index] = time(NULL);
		cgiOnGoing[index] = 0;
		for (size_t k = 0; k < pids.size(); k++)
		{
			if (index == pids.at(k).second)
			{
				kill(pids.at(k).first, 9);
				// gets rid of the temp file that child made
				std::string serverName = serverList.at(serverIndex.at(k).second).getRootDir();
				serverName.append("temp");
				serverName.append(std::to_string(index));
				unlink(serverName.c_str());
				pids.erase(pids.begin() + k);
			}
		}
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
		else if (receivedData.find("HEAD") != std::string::npos || receivedData.find("PUT") != std::string::npos ||
				 receivedData.find("CONNECT") != std::string::npos || receivedData.find("OPTIONS") != std::string::npos ||
				 receivedData.find("TRACE") != std::string::npos || receivedData.find("PATCH") != std::string::npos)
		{
			std::cout << "OTHER METHOD" << std::endl;
			handleOther(receivedData, fds, index);
		}
		else
		{
			std::cout << "HANDLING CONTINUE" << std::endl;
			handleContinue(receivedData, index);
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