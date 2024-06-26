// --CGI handling--
#include "manager.hpp"


// Timeout handling
void Manager::handleTimeout(size_t index, int k)
{
	std::string header = "HTTP/1.1 500 Internal Server Error \r\n";
	std::string body = "Server is unable to process your request in time";
	header.append("Content-Length: " + std::to_string(body.size()) + "\r\n");
	header.append("Content-Type: text/plain\r\n");
	header.append("\r\n");
	std::string response;
	response.append(header);
	response.append(body);
	size_t sendMessage = send(fds[index].fd, response.c_str(), response.length(), 0);
	if (!checkCommunication(sendMessage, index))
		return;
	std::cout << "Send return = " << sendMessage << std::endl;
	cgiOnGoing[index] = 0;
	kill(pids.at(k).first, 9);
	pids.erase(pids.begin() + k);
	std::string fullName = serverList.at(serverIndex.at(k).second).getRootDir();
	fullName.append("temp");
	fullName.append(std::to_string(index));
	unlink(fullName.c_str());
}

// Work done handling
void Manager::handleWorkDone(size_t index, int k)
{
	std::string temp = "temp";
	std::string fullName = serverList.at(serverIndex.at(k).second).getRootDir();
	temp.append(std::to_string(index));
	fullName.append(temp);
	std::string response = serverList.at(serverIndex.at(k).second).buildHTTPResponse(temp, "");
	unlink(fullName.c_str());
	size_t sendMessage = send(fds[index].fd, response.c_str(), response.length(), 0);
	if (!checkCommunication(sendMessage, index))
		return;
	cgiOnGoing[index] = 0;
	pids.erase(pids.begin() + k);
}
// CGI work handling
void Manager::handleCgiWork(size_t index)
{
	for (size_t k = 0; k < pids.size(); k++)
	{
		if (time(NULL) - fdsTimestamps[index] > RESPONSE_TIMEOUT)
		{
			for (size_t j = 0; j < serverIndex.size(); j++)
			{
				if (serverIndex.at(j).first == fds[index].fd)
				{
					handleTimeout(index, k);
					if (k == 0)
						break;
					k--;
				}
			}
		}
	}
	int status;
	int deadChildPid = waitpid(0, &status, WNOHANG);
	if (deadChildPid > 0)
	{
		for (size_t k = 0; k < pids.size(); k++)
		{
			if (deadChildPid == pids.at(k).first)
			{
				for (size_t j = 0; j < serverIndex.size(); j++)
				{
					if (serverIndex.at(j).first == fds[index].fd)
					{
						handleWorkDone(index, k);
						if (k == 0)
							break;
						k--;
					}
				}
			}
		}
	}
}