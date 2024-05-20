#include "../incl/server.hpp"

Server::Server()
{
	this->numPorts = 0;
	this->fdsSize = 0;
	this->servName = DEFAULTSERVNAME;
	this->ports.push_back(DEFAULTPORT);
	this->error404Dir = DEFAULT404DIR;
	this->cgiExt = "";
	this->cgiPath = "";
}

Server::~Server()
{
	std::cout << "Deleting server" << std::endl;
}

Server::Server(const Server &var)
{
	this->numPorts = var.numPorts;
	this->ports = var.ports;
	this->fdsSize = var.fdsSize;
	this->socketList = var.socketList;
	this->servName = var.servName;
	this->error404Dir = var.error404Dir;
	this->cgiExt = var.cgiExt;
	this->cgiExt = var.cgiPath;
}

Server &Server::operator=(const Server &var)
{
	if (this !=  &var)
	{
		this->numPorts = var.numPorts;
		this->ports = var.ports;
		this->fdsSize = var.fdsSize;
		this->socketList = var.socketList;
		this->servName = var.servName;
		this->error404Dir = var.error404Dir;
		this->cgiExt = var.cgiExt;
		this->cgiExt = var.cgiPath;
	}
	return (*this);
}

std::string Server::getMIMEType(std::string fileExt) {
    if (fileExt.compare(".html") == 0 || fileExt.compare(".htm") == 0) 
	{
        return "text/html";
    } else if (fileExt.compare(".txt") == 0) 
	{
        return "text/plain";
    } else if (fileExt.compare(".jpg") == 0 || fileExt.compare(".jpeg") == 0) 
	{
        return "image/jpeg";
    } else if (fileExt.compare(".png") == 0) 
	{
        return "image/png";
    } else 
	{
        return "application/octet-stream";
    }
}

int Server::readConfig(std::string fileName)
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
	while(1)
	{
		std::getline(configFile, line);
		// std::cout << line << std::endl;
		if (!line.compare("\0") || configFile.eof())
			break;
		// removes comments in config file
		if (line.find("#") != std::string::npos)
		{
			end = line.find("#");
			line = line.substr(0, end);
		}
		// sets port(s) if config file has any
		if (line.find("listen") != std::string::npos)
		{
			start = line.find("listen") + 7;
			wip = line.substr(start);
			end = wip.find_first_not_of("0123456789");
			wip = wip.substr(0, end);
			if (!wip.empty())
			{
				int i;
				for (i = 0; i < numPorts; i++)
				{
					if (ports.at(i) == std::stoi(wip))
						break ;
				}
				if (i == numPorts)
				{
					if (i == 0)
						ports.at(i) = std::stoi(wip);
					else
						ports.push_back(std::stoi(wip));
					numPorts++;
				}
			}
		}
		if (line.find("server_name") != std::string::npos)
		{
			start = line.find("server_name") + 12;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			if (wip != servName)
			{
				servName = wip;
			}
			//debugging help
			std::cout << wip << std::endl;
		}
		if (line.find("root") != std::string::npos)
		{
			start = line.find("root") + 5;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			rootDir = wip;
			//debugging help
			std::cout << wip << std::endl;
		}
		if (line.find("error_page 404") != std::string::npos)
		{
			start = line.find("error_page 404") + 15;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			error404Dir = wip;
			//debugging help
			std::cout << wip << std::endl;
		}
		if (line.find("cgi_ext") != std::string::npos)
		{
			start = line.find("cgi_ext") + 8;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			if (cgiExt.compare("") == 0)
				cgiExt = wip;
			//debugging help
			std::cout << wip << std::endl;
		}
		if (line.find("cgi_path") != std::string::npos)
		{
			start = line.find("cgi_path") + 9;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			if (cgiPath.compare("") == 0)
				cgiPath = wip;
			//debugging help
			std::cout << wip << std::endl;
		}
		
	}
	//debugging help
	std::cout << "cgi path = " << cgiPath << std::endl;
	std::cout << "cgi ext = " << cgiExt << std::endl;
	return 1;
}

std::string Server::buildHTTPResponse(std::string fileName, std::string fileExt)
{
	std::string response;
	std::string mimeType = getMIMEType(fileExt);
	std::cout << "file name = " << fileName << std::endl;
	std::string header;
	std::string buffer;
	std::stringstream headerStream;
	std::stringstream responseStream;

	// if empty, aka front page
	if (fileName.empty())
	{
		// responseStream << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/plain\r\n" << "\r\n" << "Front page";
		responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nFront page";
		response = responseStream.str();
		return response;
	}

	// if some other page
	std::string fileFull;
	fileFull.append(rootDir);
	fileFull.append(fileName);
	fileFull.append(fileExt);
	std::cout << fileFull << std::endl;
	int fileFd = open(fileFull.data(), O_RDONLY);
	std::ifstream file(fileFull);
	if (file.is_open() == 0)
	{
		std::string errorDir;
		errorDir.append(rootDir);
		errorDir.append(error404Dir);
		std::ifstream errormsg(errorDir);
		if (errormsg.is_open() == 0)
		{
			responseStream << "HTTP/1.1 404 Not Found\r\n" << "Content-Length: 66\r\n" << "\r\n" << "Page you were looking for does not exist, nor should it ever exist";
			response = responseStream.str();
			return response;
		}
		std::getline(errormsg, buffer, '\0');
		headerStream << "HTTP/1.1 404 Not Found\r\n" << "Content-Length: " << buffer.size() << "\r\n" << "\r\n";
		header = headerStream.str();
		response.append(header);
		response.append(buffer);
		return response;
	}
	// headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Type: " << mimeType << "\r\n" << "\r\n";
	std::getline(file, buffer, '\0');
	headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << buffer.size() << "\r\n" << "\r\n";
	header = headerStream.str();
	response.append(header);
	response.append(buffer);
	std::cout << "response = " <<response << std::endl;
	return response;
}

void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Server::makeSocket(int port)
{
	listeningSocket newSocket(port);
    setNonBlocking(newSocket.getServerFd()); // Set the listening socket to non-blocking mode
    this->socketList.push_back(newSocket); // Add the new socket to the list
    std::cout << "Socket for port " << port << " created and added to the list." << std::endl;

}

void Server::log(std::string text)
{
	std::ofstream logfile;
	logfile.open("logfile.txt", std::ofstream::app);
	if (logfile.is_open() == 0)
	{
		std::cout << "Failed to open logfile.txt" << std::endl;
		return ;
	}
	time_t rawtime;
	struct tm *timeinfo;
	char timeBuffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeBuffer, 80, "%T %d:%m:%Y", timeinfo);
	logfile << "----------------------------------------------------------------------------------------------------" << std::endl;
	logfile << "New entry in log, at time " << timeBuffer << ":" << std::endl;
	logfile << text;
	logfile << std::endl << std::endl;
	logfile.close();
}

void Server::launch(std::string configFile)
{
	if (readConfig(configFile) == 0)
		return ;
	if (numPorts == 0)
		numPorts = 1;
	for (int i = 0; i < numPorts; i++)
	{
		std::cout << "about to make a socket , port num = " << ports.at(i) << std::endl;
		makeSocket(ports.at(i));
	}
	std::vector<struct pollfd> fds;

    // Add listening sockets to pollfd structure
    for (size_t i = 0; i < socketList.size(); ++i) 
	{
        struct pollfd pfd;
        pfd.fd = socketList[i].getServerFd();
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

        for (size_t i = 0; i < fds.size(); ++i) 
		{
            if (fds[i].revents & POLLIN) 
			{
                if (fds[i].fd == socketList[i].getServerFd()) 
				{
                    // Accept new client connections
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
                        // Add the new client socket to the pollfd structure
                        struct pollfd pfd;
                        pfd.fd = clientFd;
                        pfd.events = POLLIN;
                        fds.push_back(pfd);

                        std::cout << "New client connected" << std::endl;
                    }
                } else 
				{
                    // Handle data from an existing client
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
						log(receivedData);
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
						if (fileExt.compare(".php") == 0)
						{
							std::cout << "CGI" << std::endl;
							if (cgiExt.compare("") == 0 || cgiPath.compare("") == 0)
							{
								std::cout << "ERROR, CGI not set" << std::endl;
							}
							// do CGI 
						}
						else
						{
							response = buildHTTPResponse(fileName, fileExt);
                        	send(fds[i].fd, response.c_str(), response.length(), 0);
						}
						std::cout << "prev message from this client was " << time(NULL) - socketList[0].getTimeOfLastMsg() << " seconds ago" << std::endl;
                    }
                }
            }
        }
		if (pollCount == 0)
		{
			std::cout << "timeout" << std::endl;
		}
    }
}