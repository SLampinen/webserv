#include "manager.hpp"

bool Manager::checkCommunication(int communication, size_t index)
{
	if (communication == -1)
	{
		closeConnection(index, "Communication error: " + std::string(strerror(errno)));
		return false;
	}
	else if (communication == 0)
	{
		closeConnection(index, "Connection closed by client");
		return false;
	}
	else
		return true;
}

void Manager::closeConnection(size_t &index, std::string message)
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
		closeConnection(index, "Closing connection due to inactivity");
}

void Manager::handleClientCommunication(size_t index)
{
	char buffer[1024];
	int bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
	if (!checkCommunication(bytesReceived, index))
		return;
	else
	{
		std::string receivedData(buffer, bytesReceived);
		// bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);

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
		// while (bytesReceived > 0)
		// {
		// 	std::string rest(buffer, bytesReceived);
		// 	receivedData.append(rest);
		// 	bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		// 	std::cout << "\n\nbytes received is:" << bytesReceived << std::endl;
		// }

		// if (receivedData.find("Content-Length:") != std::string::npos && bytesReceived != -1)
		// {
		// 	std::string contentLength = receivedData.substr(receivedData.find("Content-Length:") + 16);
		// 	contentLength = contentLength.substr(0, contentLength.find("\r\n"));
		// 	size_t length = std::stoi(contentLength);
		// 	bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		// 	while (receivedData.size() < length)
		// 	{
		// 		std::string rest(buffer, bytesReceived);
		// 		receivedData.append(rest);
		// 		length -= bytesReceived;
		// 		bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
		// 	}
		// }
		if (receivedData.find("Expect:") != std::string::npos)
		{
			std::string responsestr;
			std::stringstream responseStream;
			responseStream << "HTTP/1.1 413 Request Entity Too Large\r\nContent-Length: 24\r\n\r\nRequest Entity Too Large";
			responsestr = responseStream.str();
			size_t sendMessage = send(fds[index].fd, responsestr.c_str(), responsestr.length(), 0);
			if (!checkCommunication(sendMessage, index))
				return;
			closeConnection(index, "File too big");
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
		if (receivedData.find("GET") != std::string::npos)
		{
			handleGet(receivedData, fds, index);
		}
		else if (receivedData.find("POST") != std::string::npos)
		{
			handlePost(receivedData, fds, index);
		}
		else if (receivedData.find("DELETE") != std::string::npos)
		{
			handleDelete(receivedData, fds, index);
		}
		else if ((receivedData.find("HEAD") != std::string::npos || receivedData.find("PUT") != std::string::npos ||
				  receivedData.find("CONNECT") != std::string::npos || receivedData.find("OPTIONS") != std::string::npos ||
				  receivedData.find("TRACE") != std::string::npos || receivedData.find("PATCH") != std::string::npos) &&
				 receivedData.find("Content-Type:") == std::string::npos)
		{
			handleOther(receivedData, fds, index);
		}
		else
		{
			handleContinue(receivedData, index);
		}
	}
}
