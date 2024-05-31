#include "../incl/manager.hpp"

Manager::Manager()
{
	numOfServers = 0;
}

Manager::~Manager()
{

}

Manager::Manager(const Manager &var)
{
	this->numOfServers = var.numOfServers;
	this->serverList = var.serverList;
	this->serverIndex = var.serverIndex;
	this->timers = var.timers;
	this->data = var.data;
}

Manager &Manager::operator=(const Manager &var)
{
	if (this != &var)
	{
		this->numOfServers = var.numOfServers;
		this->serverList = var.serverList;
		this->serverIndex = var.serverIndex;
		this->timers = var.timers;
		this->data = var.data;
	}
	return (*this);
}

void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Manager::handleGet(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	std::string response;
	size_t start = receivedData.find('/');
	size_t end = receivedData.find(' ', start);
	std::string file = receivedData.substr(start + 1, end - start - 1);
	std::string fileExt = ".html";
	std::string fileName;
	if (file.find('.') != std::string::npos)
	{
		fileExt = file.substr(file.find('.'));
		fileName = file.substr(0, file.find('.'));
	}
	else
		fileName = file;
	std::cout << "File ext = " << fileExt << std::endl;
	if (fileExt.find("?") != std::string::npos)
	{
		end = fileExt.find("?");
		fileExt = fileExt.substr(0, end);
	}
	int index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}
	std::cout << "HERE!" << std::endl;
	std::cout << "File ext = " << fileExt << " and cgi ext = " << serverList.at(serverIndex.at(index).second).getCGIExt() << std::endl;
	std::cout << serverList.size() << " and " << serverIndex.size() << " and " <<index << std::endl;
	if (fileExt.compare(serverList.at(serverIndex.at(index).second).getCGIExt()) == 0)
	{
		std::cout << "starting CGI" << std::endl;
		handleCGI(receivedData, fds, i);
	}
	else
	{
		//for debugging
		std::cout << "here, making up a response, i = " << i << std::endl;
		serverList.at(serverIndex.at(index).second).log(receivedData);
		response = serverList.at(serverIndex.at(index).second).buildHTTPResponse(fileName, fileExt);
		std::cout << "The response is :" << std::endl << response << std::endl;

		if (serverList.at(serverIndex.at(index).second).getClientBodySize() != 0 && serverList.at(serverIndex.at(index).second).getClientBodySize() < response.length())
		{
			std::stringstream responseStream;
			std::cout << "response too large" << std::endl;
			responseStream << "HTTP/1.1 413 Request Entity Too Large\r\n"
						   << "Content-Length: 34\r\n"
						   << "\r\n"
						   << "ERROR 413 Request Entity Too Large";
			response = "";
			response = responseStream.str();
		}
		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
}

void Manager::handlePost(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	for (int j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}
	int start = receivedData.find("data=") + 5;
	std::string wip = receivedData.substr(start);
	// std::cout << "data = " << wip << std::endl;
	if (wip.compare("add") == 0)
		data.push_back(wip);
	if (wip.compare("remove") == 0)
	{
		if (data.size() > 0)
			data.pop_back();
	}
	if (wip.compare("print") == 0)
	{
		for (int i = 0; i < data.size(); i++)
		{
			std::cout << data.at(i).data() << std::endl;
		}
	}
	// the following just send bogus response to see if it works
		std::string response;
		std::string buffer;
		std::ifstream file("www/result.html");
		std::getline(file, buffer, '\0');
		std::stringstream headerStream;
		headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << buffer.size() << "\r\n" << "\r\n";
		std::string header = headerStream.str();
		response.append(header);
		response.append(buffer);
		send(fds[i].fd, response.c_str(), response.length(), 0);
}

void Manager::handleDelete(std::string receivedData, std::vector <struct pollfd> fds, int i)
{

}

void Manager::run(std::string configFile) 
{
    if (readConfig(configFile) == 0) 
	{
        std::cerr << "ERROR reading config file" << std::endl;
        return;
    }
    std::cout << "--------------------------------------------------" << std::endl
              << "number of servers = " << numOfServers << std::endl;

    for (int i = 0; i < numOfServers; i++) 
	{
        serverList.at(i).print();
        for (int k = 0; k < serverList.at(i).getNumOfPorts(); k++) 
		{
            struct pollfd pfd;
            pfd.fd = serverList.at(i).listeners.at(k).getServerFd();
            pfd.events = POLLIN;
            fds.push_back(pfd);
        }
    }
    while (true) 
	{
        int pollCount = poll(fds.data(), fds.size(), POLL_TIMEOUT);
        if (pollCount < 0) 
		{
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            continue;
        }
        for (size_t i = 0; i < fds.size(); i++) 
		{
            if (fds[i].revents & POLLIN) 
			{
                // Iterate over each server and its listeners to find the matching serverFd
                bool newConnection = false;
                for (int j = 0; j < numOfServers; j++) 
				{
                    for (int k = 0; k < serverList.at(j).getNumOfPorts(); k++) 
					{
                        if (fds[i].fd == serverList.at(j).listeners.at(k).getServerFd()) 
						{
                            std::cout << "new connection" << std::endl;
                            while (true) 
							{
                                struct sockaddr_in clientAddr;
                                socklen_t clientAddrLen = sizeof(clientAddr);
                                int clientFd = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                                if (clientFd < 0) 
								{
                                    if (errno == EWOULDBLOCK || errno == EAGAIN) 
									{
                                        // No more clients to accept
                                        break;
                                    } else 
									{
                                        std::cerr << "Accept error: " << strerror(errno) << std::endl;
                                        break;
                                    }
                                }
                                setNonBlocking(clientFd);
                                // Add the new client socket to the poll list
                                struct pollfd clientPfd;
                                clientPfd.fd = clientFd;
                                clientPfd.events = POLLIN;
                                fds.push_back(clientPfd);
								serverIndex.push_back(std::make_pair(clientFd, j));
								std::cout << "here, i, j, k are "<< i << " " << j << " " << k << std::endl;
                            }
                            newConnection = true;
                            break;
                        }
                    }
                    if (newConnection) break;
                }
                if (!newConnection) {
                    // Handle communication with the client
                    char buffer[1024];
                    int bytesReceived = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (bytesReceived < 0)
					{
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
						{
                            // No data available to read
                            continue;
                        } else
						{
                            std::cerr << "Recv error: " << strerror(errno) << std::endl;
                            close(fds[i].fd);
                            fds.erase(fds.begin() + i);
                            i--; // Adjust index after erasing
                            continue;
                        }
                    } else if (bytesReceived == 0)
					{
                        // Connection closed by client
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--; // Adjust index after erasing
                        continue;
                    } else
					{
                        // Process the received data
                        std::string receivedData(buffer, bytesReceived);
                        std::cout << "Received data: " << std::endl << receivedData << std::endl;
						if (receivedData.find("GET") != std::string::npos)
						{
							std::cout << "GETTING" << std::endl;
							handleGet(receivedData, fds, i);
						}
						if (receivedData.find("POST") != std::string::npos)
						{
							std::cout << "POSTING" << std::endl;
							handlePost(receivedData, fds, i);
						}
						if (receivedData.find("DELETE") != std::string::npos)
						{
							std::cout << "DELETING" << std::endl;
							handleDelete(receivedData, fds, i);
						}
                    }
                }
            }
        }
    }
}

int Manager::readConfig(std::string fileName)
{

	std::fstream configFile;
	int start, end;
	std::string wip;

	configFile.open(fileName);
	if (!configFile.good())
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return 0;
	}
	std::string line;
	while (1)
	{
		std::getline(configFile, line);
		if (configFile.eof())
			break;
		// std::cout << "line is : " << line << std::endl;
		//remove comments
		if (line.find("#") != std::string::npos)
		{
			end = line.find("#");
			line = line.substr(0, end);
		}
		if (line.find("server {") != std::string::npos)
		{
			std::cout << "----------start of server block----------" << std::endl;
			std::string serverName = "server.num";
			serverName.append(std::to_string(numOfServers));
			Server newServer;
			std::cout << serverName << std::endl;
			std::cout << "The (default) name of server is " << newServer.getServerName() << std::endl;
			numOfServers++;
			while (1)
			{
				std::getline(configFile, line);
				std::cout << "line in block is : " << line << std::endl;
				if (configFile.eof())
					break;
				if (line.find("#") != std::string::npos)
				{
					end = line.find("#");
					line = line.substr(0, end);
				}
				if (line.find("}") != std::string::npos)
				{
					std::cout << "----------end of block----------" << std::endl;
					break;
				}
				if (line.find("server_name") != std::string::npos)
				{
					start = line.find("server_name") + 12;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setServerName(wip);
				}
				if (line.find("listen") != std::string::npos)
				{
					start = line.find("listen") + 7;
					wip = line.substr(start);
					end = wip.find_first_not_of("0123456789");
					wip = wip.substr(0, end);
					if (!wip.empty())
					{
						std::cout << "setting port" << std::endl;
						newServer.setPort(std::stoi(wip));
						newServer.addPort(std::stoi(wip));
					}
				}
				if (line.find("root") != std::string::npos)
				{
					start = line.find("root") + 5;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setRootDir(wip);
				}
				if (line.find("error_page 404") != std::string::npos)
				{
					start = line.find("error_page 404") + 15;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setErrorDir(wip);
				}
				if (line.find("cgi_ext") != std::string::npos)
				{
					start = line.find("cgi_ext") + 8;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setCGIExt(wip);
				}
				if (line.find("cgi_path") != std::string::npos)
				{
					start = line.find("cgi_path") + 9;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setCGIPath(wip);
				}
				if (line.find("client_max_body_size") != std::string::npos)
				{
					start = line.find("client_max_body_size") + 21;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setClientBodySize(wip);
				}
				
			}
			newServer.makeSocketList();
			serverList.push_back(newServer);
			std::cout << "end of server block" <<std::endl;
		}
	}
	return 1;
}

void Manager::handleCGI(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	std::string response;
	for (int j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			if (serverList.at(serverIndex.at(j).second).getCGIExt().empty() || serverList.at(serverIndex.at(j).second).getCGIPath().empty())
			{
				std::cout << "ERROR, CGI not set" << std::endl;
				std::stringstream responseStream;
				responseStream << "HTTP/1.1 418 I'm a teapot\r\n" << "Content-Length: 17\r\n" << "\r\n" << "CGI not available";
				response = responseStream.str();
			}
			else
			{
				std::cout << "Doing cgi" << std::endl;
				int pid;
				pid = fork();
				if (pid == 0)
				{
					std::string path = serverList.at(serverIndex.at(j).second).getCGIPath();
					std::string cmd = serverList.at(serverIndex.at(j).second).getRootDir();
					receivedData = receivedData.substr(receivedData.find("/") + 1);
					cmd.append(receivedData);
					if (cmd.find("?") != std::string::npos)
					{
						int end = cmd.find("?");
						cmd.erase(end);
					}
					std::cout << "path = " <<path << std::endl;
					std::cout << "cmd = " <<cmd << std::endl; 
					char *cmdArr[] = {const_cast<char *>(path.data()), const_cast<char *>(cmd.data()), NULL};
					execvp(cmdArr[0], cmdArr);
					exit(0) ;
				}
			}
			break;
		}
	}
	std::cout << "Sending response : " << std::endl << response << std::endl;
	send(fds[i].fd, response.c_str(), response.length(), 0);
}