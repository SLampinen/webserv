#include "../incl/server.hpp"

Server::Server()
{
	this->socketList = listeningSocket(DEFAULTPORT);
	this->servName = "defaultserv";
	this->port = DEFAULTPORT;
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
	this->port = var.port;
	this->socketList = var.socketList;
	this->servName = var.servName;
	this->rootDir = var.rootDir;
	this->error404Dir = var.error404Dir;
	this->cgiExt = var.cgiExt;
	this->cgiPath = var.cgiPath;
}

Server &Server::operator=(const Server &var)
{
	if (this !=  &var)
	{
		this->port = var.port;
		this->socketList = var.socketList;
		this->servName = var.servName;
		this->rootDir = var.rootDir;
		this->error404Dir = var.error404Dir;
		this->cgiExt = var.cgiExt;
		this->cgiPath = var.cgiPath;
	}
	return (*this);
}

void Server::print(void)
{
	std::cout << "server name = " << servName << std::endl;
	std::cout << "root dir = " << rootDir << std::endl;
	std::cout << "error dir = " << error404Dir << std::endl;
	std::cout << "cgi path and ext  = " << cgiPath << " and " << cgiExt << std::endl;
	std::cout << "port is " << port << std::endl;
}

std::string Server::getServerName(void)
{
	return this->servName;
}

void Server::setServerName(std::string name)
{
	this->servName = name;
}

int Server::getPort(void)
{
	return this->port;
}

void Server::setPort(int portNum)
{
	this->port = portNum;
}

void Server::setRootDir(std::string dir)
{
	this->rootDir = dir;
}

void Server::setErrorDir(std::string dir)
{
	this->error404Dir = dir;
}

void Server::setCGIExt(std::string ext)
{
	this->cgiExt = ext;
}

void Server::setCGIPath(std::string path)
{
	this->cgiPath = path;
}

std::string Server::getCGIPath(void)
{
	return this->cgiPath;
}

std::string Server::getCGIExt(void)
{
	return this->cgiExt;
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
		std::string fileFull;
		fileFull.append(rootDir);
		fileFull.append("home.html");
		std::cout << "the front page is " << fileFull << std::endl;
		int fileFd = open(fileFull.data(), O_RDONLY);
		std::ifstream file(fileFull);
		if (file.is_open() == 0)
		{
			responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nFront page";
			response = responseStream.str();
			return response;
		}
		std::getline(file, buffer, '\0');
		headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << buffer.size() << "\r\n" << "\r\n";
		header = headerStream.str();
		response.append(header);
		response.append(buffer);
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

void setnonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Server::makeSocket(int port)
{
	listeningSocket newSocket(port);
    setnonblocking(newSocket.getServerFd()); // Set the listening socket to non-blocking mode
    this->socketList = newSocket; // Add the new socket to the list
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

void Server::launch()
{
	makeSocket(port);
	std::vector<struct pollfd> fds;

    // Add listening socket to pollfd structure
	struct pollfd pfd;
	pfd.fd = socketList.getServerFd();
	pfd.events = POLLIN;
	fds.push_back(pfd);

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
                if (fds[i].fd == socketList.getServerFd()) 
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
                        setnonblocking(clientFd);
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
						std::cout << "prev message from this client was " << time(NULL) - socketList.getTimeOfLastMsg() << " seconds ago" << std::endl;
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