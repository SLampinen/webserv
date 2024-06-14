#include "../incl/manager.hpp"

Manager::Manager()
{

}

Manager::~Manager()
{

}

Manager::Manager(const Manager &var)
{
	this->pids = var.pids;
	this->serverList = var.serverList;
	this->serverIndex = var.serverIndex;
	this->fds = var.fds;
	this->cgiOnGoing = var.cgiOnGoing;
	this->fdsTimestamps = var.fdsTimestamps;
}

Manager &Manager::operator=(const Manager &var)
{
	if (this != &var)
	{
		this->pids = var.pids;
		this->serverList = var.serverList;
		this->serverIndex = var.serverIndex;
		this->fds = var.fds;
		this->cgiOnGoing = var.cgiOnGoing;
		this->fdsTimestamps = var.fdsTimestamps;
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
		serverList.at(serverIndex.at(index).second).log(receivedData);
		handleCGI(receivedData, fds, i);
	}
	else
	{
		//for debugging
		std::cout << "here, making up a response, i = " << i << std::endl;
		
		serverList.at(serverIndex.at(index).second).log(receivedData);

		response = serverList.at(serverIndex.at(index).second).buildHTTPResponse(fileName, fileExt);
		// std::cout << "The response is :" << std::endl << response << std::endl;

		if (serverList.at(serverIndex.at(index).second).getClientBodySize() != 0 && serverList.at(serverIndex.at(index).second).getClientBodySize() < response.length())
		{
			std::stringstream responseStream;
			std::cout << "response too large" << std::endl;
			std::string body = "ERROR 413 Request Entity Too Large";
			response = serverList.at(serverIndex.at(index).second).makeHeader(413, body.size());
			response.append(body);
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
	std::string path;
	int start = receivedData.find("/") + 1;
	int end = receivedData.find(" ", start);
	path = receivedData.substr(start, end - start);
	start = receivedData.find("boundary=") + 9;
	end = receivedData.find_first_not_of("-0123456789", start);
	std::string boundary = receivedData.substr(start, end - start);
	end = receivedData.find(boundary, end);
	std::string rawData = receivedData.substr(end);
	if (path.compare("upload") == 0)
	{
		handleUpload(rawData, boundary, fds, i);
	}
	else
	{
	// the following just sends bogus response to see if it works
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
}

void Manager::handleDelete(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	for (int j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}
	std::string response;
	int index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}
	std::string body = "OK";
	response = serverList.at(serverIndex.at(index).second).makeHeader(200, body.size());
	response.append(body);
	send(fds[i].fd, response.c_str(), response.length(), 0);
}

void Manager::handleOther(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	for (int j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			serverList.at(serverIndex.at(j).second).log(receivedData);
			break;
		}
	}
	std::string response;
	int index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}
	std::string body = "Method Not Implemented";
	response = serverList.at(serverIndex.at(index).second).makeHeader(501, body.size());
	response.append(body);
	send(fds[i].fd, response.c_str(), response.length(), 0);
}

void Manager::run(std::string configFile) 
{
    if (readConfig(configFile) == 0) 
	{
        std::cerr << "ERROR reading config file" << std::endl;
        return;
    }
    std::cout << "--------------------------------------------------" << std::endl
              << "number of servers = " << serverList.size() << std::endl;

    for (int i = 0; i < serverList.size(); i++) 
	{
        serverList.at(i).print();
        for (int k = 0; k < serverList.at(i).getNumOfPorts(); k++) 
		{
            struct pollfd pfd;
            pfd.fd = serverList.at(i).listeners.at(k).getServerFd();
            pfd.events = POLLIN;
            fds.push_back(pfd);
			fdsTimestamps.push_back(2147483647);
			cgiOnGoing.push_back(0);

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
		std::cout << "new round " << std::endl;
        for (size_t index = 0; index < fds.size(); index++) 
		{
            if (fds[index].revents & POLLIN) 
			{
                // Iterate over each server and its listeners to find the matching serverFd
                bool newConnection = false;
                for (int j = 0; j < serverList.size(); j++) 
				{
                    for (int k = 0; k < serverList.at(j).getNumOfPorts(); k++) 
					{
                        if (fds[index].fd == serverList.at(j).listeners.at(k).getServerFd()) 
						{
                            std::cout << "new connection" << std::endl;
                            while (true) 
							{
                                struct sockaddr_in clientAddr;
                                socklen_t clientAddrLen = sizeof(clientAddr);
                                int clientFd = accept(fds[index].fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
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
								fdsTimestamps.push_back(time(NULL));
								cgiOnGoing.push_back(0);
								serverIndex.push_back(std::make_pair(clientFd, j));
								std::cout << "here, i, j, k are "<< index << " " << j << " " << k << std::endl;
                            }
                            newConnection = true;
                            break;
                        }
                    }
                    if (newConnection) break;
                }
                if (!newConnection)
				{
                    // Handle communication with the client
                    char buffer[1024];
                    int bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
                    if (bytesReceived < 0)
					{
						//Is this legal? or is this checking errno immediatly after read operation?
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
						{
                            // No data available to read
                            continue;
                        }
						else
						{
                            std::cerr << "Recv error: " << strerror(errno) << std::endl;
                            close(fds[index].fd);
                            fds.erase(fds.begin() + index);
							fdsTimestamps.erase(fdsTimestamps.begin() + index);
							cgiOnGoing.erase(cgiOnGoing.begin() + index);
                            index--; // Adjust index after erasing
                            continue;
                        }
                    }
					else if (bytesReceived == 0)
					{
                        // Connection closed by client
						std::cout << std::endl << "Client closed connection" << std::endl;
                        close(fds[index].fd);
                        fds.erase(fds.begin() + index);
						fdsTimestamps.erase(fdsTimestamps.begin() + index);
						cgiOnGoing.erase(cgiOnGoing.begin() + index);
                        index--; // Adjust index after erasing
                        continue;
                    }
					else
					{
						//if request too large
                        std::string receivedData(buffer, bytesReceived);
						bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
						while (bytesReceived > 0)
						{
							std::string rest(buffer, bytesReceived);
							receivedData.append(rest);
							bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
						}

						// If cgi was already working, we can just ignore it and move on
						fdsTimestamps[index] = time(NULL);
						cgiOnGoing[index] = 0;
						for (int k = 0; k < pids.size(); k++)
						{
							if (index == pids.at(k).second)
								pids.erase(pids.begin() + k);
						}
						std::cout << "NEW MESSAGE" << std::endl;
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
                    }
                }
            }
			if (cgiOnGoing[index] == 1)
			{
				//check if cgi has done the work and if yes send response
				std::cout << "Checking and working and all that for i = " << index << std::endl;
				int status;
				int deadChildPid = waitpid(0, &status, WNOHANG);
				//check which, if any have work done
				if (deadChildPid > 0)
				{
					std::cout << "Child with pid " << deadChildPid << " is dead" << std::endl;
					for (int k = 0; k < pids.size(); k++)
					{
						if (time(NULL) - fdsTimestamps[index] > RESPONSE_TIMEOUT)
						{
							int j;
							for (j = 0; j < serverIndex.size(); j++)
							{
								if (serverIndex.at(j).first == fds[index].fd)
								{
									std::cout << "This should timeout, i = " << index << " and fd = " << fds[index].fd << std::endl;
									std::cout << "Last message happened at" << fdsTimestamps[index] << std::endl;
									std::string body = "Connection timeout";
									std::string response = serverList.at(serverIndex.at(j).second).makeHeader(500, body.size());
									response.append(body);
									send(fds[index].fd, response.c_str(), response.length(), 0);
									cgiOnGoing[index] = 0;
									pids.erase(pids.begin() + k);
									if (k == 0)
										break;
									k--;
									std::string fullName = serverList.at(serverIndex.at(j).second).getRootDir();
									fullName.append("temp");
									unlink(fullName.c_str());
								}
							}
						}
						else if (deadChildPid == pids.at(k).first)
						{
							std::cout << "THE BEGINNING" << std::endl;
							std::cout << "pid here = " << pids.at(k).first << " and result = " << deadChildPid << std::endl;
							int j;
							for (j = 0; j < serverIndex.size(); j++)
							{
								if (serverIndex.at(j).first == fds[index].fd)
								{
									std::cout << "Working" << std::endl;
									std::string temp = "temp";
									std::string fullName = serverList.at(serverIndex.at(j).second).getRootDir();
									temp.append(std::to_string(index));
									fullName.append(temp);
									std::cout << "Full name (including directory) = " << fullName << std::endl;
									std::cout << "Name of temp file = " << temp << std::endl;
									std::cout << "pid here = " << pids.at(k).first << " and result = " << deadChildPid << std::endl;
									std::string response = serverList.at(serverIndex.at(j).second).buildHTTPResponse(temp, "");
									unlink(fullName.c_str());
									std::cout << "Request arrived " << time(NULL) - fdsTimestamps[index] << " seconds ago" << std::endl;
									send(fds[index].fd, response.c_str(), response.length(), 0);
									cgiOnGoing[index] = 0;
									std::cout << "k = " << k << std::endl;
									std::cout << "pids.size() = " << pids.size() << std::endl;
									pids.erase(pids.begin() + k);
									if (k == 0)
										break;
									k--;
								}
							}
						}
					}
				}
			}
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
		if (line.find("#") != std::string::npos)
		{
			end = line.find("#");
			line = line.substr(0, end);
		}
		if (line.find("server {") != std::string::npos)
		{
			std::cout << "----------start of server block----------" << std::endl;
			std::string serverName = "server.num";
			serverName.append(std::to_string(serverList.size()));
			Server newServer;
			std::cout << serverName << std::endl;
			std::cout << "The (default) name of server is " << newServer.getServerName() << std::endl;
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
			std::cout << "end of server block" <<std::endl;
			if (newServer.getNumOfPorts() > 0)
			{
				newServer.makeSocketList();
				serverList.push_back(newServer);
			}
			else
			{
				std::cout << "ERROR: Server has 0 ports" << std::endl;
			}
		}
	}
	if (serverList.size() == 0)
	{
		std::cout << "ERROR: Config file has no servers" << std::endl;
		return 0;
	}
	return 1;
}

void Manager::handleCGI(std::string receivedData, std::vector <struct pollfd> fds, int i)
{
	std::string response;
	cgiOnGoing[i] = 1;
	for (int j = 0; j < serverIndex.size(); j++)
	{
		if (serverIndex.at(j).first == fds[i].fd)
		{
			if (serverList.at(serverIndex.at(j).second).getCGIExt().empty() || serverList.at(serverIndex.at(j).second).getCGIPath().empty())
			{
				std::cout << "ERROR, CGI not set" << std::endl;
				std::string body = "CGI not available";
				response = serverList.at(serverIndex.at(j).second).makeHeader(418, body.size());
				response.append(body);
				cgiOnGoing[i] = 0;
				send(fds[i].fd, response.c_str(), response.length(), 0);
			}
			else
			{
				std::cout << "Doing cgi" << std::endl;
				int pid;
				std::cout << "FORKING" << std::endl;
				pid = fork();
				if (pid < 0)
				{
					std::cout << "ERROR piderror" << std::endl; 
					std::string body = "Internal server error";
					response = serverList.at(serverIndex.at(j).second).makeHeader(500, body.size());
					response.append(body);
					cgiOnGoing[i] = 0;
					send(fds[i].fd, response.c_str(), response.length(), 0);
				}
				else if (pid == 0)
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
					std::string fName = serverList.at(serverIndex.at(j).second).getRootDir();
					fName.append("temp");
					fName.append(std::to_string(i));
					int fd = open(fName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777);
					dup2(fd, 1);
					char *cmdArr[] = {const_cast<char *>(path.data()), const_cast<char *>(cmd.data()), NULL};
					execv(cmdArr[0], cmdArr);
					std::cerr << strerror(errno) << std::endl;
					for (size_t i = 0; cmdArr[i]; i++)
					{
						std::cerr << cmdArr[i] << std::endl;
					}
					
					exit(0) ;
				}
				else
				{
					std::cout << "pid is " << pid << std::endl;
					pids.push_back(std::make_pair(pid, i));
				}

			}
			break;
		}
	}
}

void Manager::handleUpload(std::string receivedData, std::string boundary, std::vector <struct pollfd> fds, int i)
{
	int index;
	for (index = 0; index < serverIndex.size(); index++)
	{
		if (serverIndex.at(index).first == fds[i].fd)
			break;
	}
	
	std::cout << "This is the data we got: " << receivedData << std::endl;
	int start = receivedData.find("filename=") + 10;
	int end = receivedData.find("\"", start);
	std::string name = receivedData.substr(start, end - start);

	std::cout << name << std::endl;

	std::ofstream theFile;
	std::string root = serverList.at(serverIndex.at(index).second).getRootDir().append("files/");
	name = root.append(name);
	std::cout << "name = " << name << std::endl;
	theFile.open(name);

	start = receivedData.find("Content-Type");
	start = receivedData.find("\n",start);
	end  = receivedData.find(boundary, start);

	std::string fileContent = receivedData.substr(start, end - start);
	theFile << fileContent;
	theFile.close();
	
	std::string response;
	std::stringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 26\r\n\r\nFile uploaded successfully";
	response = responseStream.str();
	send(fds[i].fd, response.c_str(), response.length(), 0);
}