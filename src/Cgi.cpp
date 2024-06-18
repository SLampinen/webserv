#include "../incl/manager.hpp"
// --CGI handling--
// Timeout handling
void Manager::handleTimeout(size_t index, int k)
{
	std::cout << "This should timeout, i = " << index << " and fd = " << fds[index].fd << std::endl;
	std::cout << "Last message happened at " << fdsTimestamps[index] << std::endl;
	std::string body = "Connection timeout";
	std::string response = serverList.at(serverIndex.at(k).second).makeHeader(408, body.size());
	std::cout << "NOW resp = " << response << std::endl;
	response.append(body);
	std::cout << "AFTER resp = " << response << std::endl;
	std::cout << "Send return = " << send(fds[index].fd, response.c_str(), response.length(), 0) << std::endl;
	cgiOnGoing[index] = 0;
	std::cout << "Killed at timeout" << std::endl;
	kill(pids.at(k).first, 9);
	pids.erase(pids.begin() + k);
	std::string fullName = serverList.at(serverIndex.at(k).second).getRootDir();
	fullName.append("temp");
	fullName.append(std::to_string(index));
	unlink(fullName.c_str());
	exit ;
}

// Work done handling
void Manager::handleWorkDone(size_t index, int k)
{
	std::cout << "Working" << std::endl;
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
	for (int k = 0; k < pids.size(); k++)
	{
		std::cout << time(NULL) - fdsTimestamps[index] << std::endl;
		if (time(NULL) - fdsTimestamps[index] > RESPONSE_TIMEOUT)
		{
			for (int j = 0; j < serverIndex.size(); j++)
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
		for (int k = 0; k < pids.size(); k++)
		{
			if (deadChildPid == pids.at(k).first)
			{
				for (int j = 0; j < serverIndex.size(); j++)
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