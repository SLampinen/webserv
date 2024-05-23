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
}

Manager &Manager::operator=(const Manager &var)
{
	if (this != &var)
	{
		this->numOfServers = var.numOfServers;
		this->serverList = var.serverList;
		this->serverIndex = var.serverIndex;
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
	if (fileExt.compare(".php") == 0)
	{
		std::cout << "CGI" << std::endl;
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
					//do CGI
				}
				break;
			}
		}
		std::cout << "Sending response : " << std::endl << response << std::endl;
		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
	else
	{
		std::cout << "here, making up a response, i = " << i << std::endl;
		for (int j = 0; j < serverIndex.size(); j++)
		{
			if (serverIndex.at(j).first == fds[i].fd)
			{
				serverList.at(serverIndex.at(j).second).log(receivedData);
				response = serverList.at(serverIndex.at(j).second).buildHTTPResponse(fileName, fileExt);
				break;
			}
		}
		send(fds[i].fd, response.c_str(), response.length(), 0);
	}
}

void Manager::run(std::string configFile)
{
	if (readConfig(configFile) == 0)
	{
		std::cout << "ERROR reading config file" << std::endl;
		return ;
	}
	std::cout << "--------------------------------------------------" << std::endl << "number of servers = " << numOfServers << std::endl;
	std::vector <struct pollfd> fds;
	for (int i = 0; i < numOfServers; i++)
	{
		serverList.at(i).print();
		serverList.at(i).makeSocket(serverList.at(i).getPort());
		struct pollfd pfd;
		pfd.fd = serverList.at(i).socketList.getServerFd();
		pfd.events = POLLIN;
		fds.push_back(pfd);
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
				if (fds[i].fd == serverList[i].socketList.getServerFd())
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
                        struct pollfd pfd;
                        pfd.fd = clientFd;
                        pfd.events = POLLIN;
                        fds.push_back(pfd);
                        std::cout << "New client connected, fd = " << clientFd << " and i = " << i << std::endl;
						serverIndex.push_back(std::make_pair(clientFd, i));
					}
				}
				else
				{
					char buffer[1024];
                    int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (bytesRead < 0)
					{
						// is this legal? can we check errno here? or do we need some other way to do this?
                        if (errno != EWOULDBLOCK && errno != EAGAIN) 
						{
                            std::cerr << "Recv error: " << strerror(errno) << std::endl;
                            close(fds[i].fd);
                            fds.erase(fds.begin() + i);
                            --i;
                        }
					}
					else if (bytesRead == 0)
					{
                        // Connection closed
                        std::cout << "Client disconnected" << std::endl;
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
					}
					else 
					{
                        // Process the received data
						std::string response;
                        std::string receivedData(buffer, bytesRead);
						if (receivedData.find("GET") != std::string::npos)
						{
							std::cout << "GETTING" << std::endl;
							handleGet(receivedData, fds, i);
						}
						if (receivedData.find("POST") != std::string::npos)
						{
							std::cout << "POSTING" << std::endl;
						}
						if (receivedData.find("DELETE") != std::string::npos)
						{
							std::cout << "DELETING" << std::endl;
						}
						// std::cout << "prev message from this client was " << time(NULL) - socketList.getTimeOfLastMsg() << " seconds ago" << std::endl;
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
		std::cout << line << std::endl;
		if (!line.compare("\0") || configFile.eof())
			break;
		//remove comments
		if (line.find("#") != std::string::npos)
		{
			end = line.find("#");
			line = line.substr(0, end);
		}
		if (line.find("server {") != std::string::npos)
		{
			std::cout << "start of server block" << std::endl;
			std::string serverName = "server.num";
			serverName.append(std::to_string(numOfServers));
			Server newServer;
			std::cout << serverName << std::endl;
			std::cout << "The (default) name of server is " << newServer.getServerName() << std::endl;
			numOfServers++;
			while (1)
			{
				std::getline(configFile, line);
				// std::cout << line << std::endl;
				if (!line.compare("\0") || configFile.eof())
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
					if (!wip.empty() && newServer.getPort() == DEFAULTPORT)
					{
						std::cout << "setting port" << std::endl;
						newServer.setPort(std::stoi(wip));
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
				
			}
			newServer.print();
			serverList.push_back(newServer);
			std::cout << "end of server block" <<std::endl;
		}
	}
	return 1;
}
