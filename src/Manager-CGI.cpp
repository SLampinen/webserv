#include "manager.hpp"
// --CGI handling--
// Timeout handling
void Manager::handleTimeout(size_t index, int k)
{
	std::cout << "This should timeout, i = " << index << " and fd = " << fds[index].fd << std::endl;
	std::cout << "Last message happened at " << fdsTimestamps[index] << std::endl;
	std::string header = "HTTP/1.1 500 Internal Server Error \r\n";
	std::string body = "Server is unable to process your request in time";
	header.append("Content-Length: " + std::to_string(body.size()) + "\r\n");
	header.append("Content-Type: text/plain\r\n");
	header.append("\r\n");
	std::string response;
	response.append(header);
	response.append(body);
	std::cout << "AFTER resp = \n" << response << std::endl;
	std::cout << "Send return = " << send(fds[index].fd, response.c_str(), response.length(), 0) << std::endl;
	cgiOnGoing[index] = 0;
	std::cout << "Killed at timeout" << std::endl;
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
	std::cout << "Working" << std::endl;
	std::cout << "index is " << index << " and k is " << k << std::endl;
	std::string temp = "temp";
	std::string fullName = serverList.at(serverIndex.at(k).second).getRootDir();
	temp.append(std::to_string(index));
	fullName.append(temp);
	std::cout << "Full name (including directory) = " << fullName << std::endl;
	std::cout << "Name of temp file = " << temp << std::endl;
	std::string response = serverList.at(serverIndex.at(k).second).buildHTTPResponse(temp, "");
	unlink(fullName.c_str());
	std::cout << "Request arrived " << time(NULL) - fdsTimestamps[index] << " seconds ago" << std::endl;
	send(fds[index].fd, response.c_str(), response.length(), 0);
	cgiOnGoing[index] = 0;
	pids.erase(pids.begin() + k);
}
// CGI work handling
void Manager::handleCgiWork(size_t index)
{
	std::cout << "Checking and working and all that for i = " << index << std::endl;
	std::cout << "pids size = " << pids.size() << std::endl;
	for (size_t k = 0; k < pids.size(); k++)
	{
		std::cout << "Time diff = " << time(NULL) - fdsTimestamps[index] << ", Response timeout = " << RESPONSE_TIMEOUT << std::endl;
		if (time(NULL) - fdsTimestamps[index] > RESPONSE_TIMEOUT)
		{
			for (size_t j = 0; j < serverIndex.size(); j++)
			{
				std::cout << "j = " << j << std::endl;
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
		std::cout << "Child with pid " << deadChildPid << " is dead" << std::endl;
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