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
	if (fileExt.compare(serverList.at(serverIndex.at(index).second).getCGIExt()) == 0)
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
		std::cout << "ERROR reading config file" << std::endl;
		return ;
	}
	std::cout << "--------------------------------------------------" << std::endl << "number of servers = " << numOfServers << std::endl;
	// std::vector <struct pollfd> fds;
	for (int i = 0; i < numOfServers; i++)
	{
		serverList.at(i).print();
		for (int k = 0; k < serverList.at(i).getNumOfPorts(); k++)
		{
			std::cout << "doing this " << std::endl;
			serverList.at(i).makeSocket(serverList.at(i).getNthPort(k));
			struct pollfd pfd;
			pfd.fd = serverList.at(i).listeners.at(k).getServerFd();
			pfd.events = POLLIN;
			fds.push_back(pfd);
			timers.push_back(std::make_pair(pfd.fd, 0));
		}
		// serverList.at(i).makeSocket(serverList.at(i).getPort());
		// struct pollfd pfd;
		// pfd.fd = serverList.at(i).listener.getServerFd();
		// pfd.events = POLLIN;
		// fds.push_back(pfd);
		// timers.push_back(std::make_pair(pfd.fd, 0));
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
				if (fds[i].fd == serverList[i].listener.getServerFd())
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
						timers.push_back(std::make_pair(pfd.fd, time(NULL)));
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
						for (int index = 0; index < timers.size(); index++)
						{
							if (timers.at(index).first == fds[i].fd)
							{
								timers.erase(timers.begin() + index);
								break;
							}
						}
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
							handlePost(receivedData, fds, i);
						}
						if (receivedData.find("DELETE") != std::string::npos)
						{
							std::cout << "DELETING" << std::endl;
							handleDelete(receivedData, fds, i);
						}
						std::cout << "Prev message from this (fd " << timers.at(i).first << ") came " << time(NULL) - timers.at(i).second << " seconds ago" << std::endl;
						timers.at(i).second = time(NULL);
					}
				}
			}
			// if (fds[i].fd != serverList[i].listener.getServerFd())
			// {
			// 	if (time(NULL) - timers.at(i).second > 5)
			// 	{
			// 		std::cout << "time now = " << time(NULL) << std::endl;
			// 		std::cout << "Connection timeout for " << fds[i].fd << std::endl;
			// 		close(fds[i].fd);
            //         fds.erase(fds.begin() + i);
			// 		for (int index = 0; index < timers.size(); index++)
			// 		{
			// 			if (timers.at(index).first == fds[i].fd)
			// 			{
			// 				timers.erase(timers.begin() + index);
			// 				break;
			// 			}
			// 		}
			// 	}
			// 	// else
			// 		// std::cout << "Still has time" << std::endl;
			// }
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
						// newServer.setPort(std::stoi(wip));
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
			serverList.push_back(newServer);
			std::cout << "end of server block" <<std::endl;
		}
	}
	return 1;
}
