#include "../incl/server.hpp"

Server::Server()
{
	this->numPorts = 0;
	this->fdsSize = 0;
	this->servName = DEFAULTSERVNAME;
	// this->port = DEFAULTPORT;
	this->ports.push_back(DEFAULTPORT);
	this->error404Dir = DEFAULT404DIR;
	std::string response;
	size_t responseLen;
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
	// this->port = var.port;
	this->servName = var.servName;
	// this->lSocket = var.lSocket;
	this->response = var.response;
	this->error404Dir = var.error404Dir;
}

Server &Server::operator=(const Server &var)
{
	if (this !=  &var)
	{
		this->numPorts = var.numPorts;
		this->ports = var.ports;
		this->fdsSize = var.fdsSize;
		this->socketList = var.socketList;
		// this->port = var.port;
		this->servName = var.servName;
		// this->lSocket = var.lSocket;
		this->response = var.response;
		this->error404Dir = var.error404Dir;
	}
	return (*this);
}

std::string Server::getMIMEType(std::string fileExt) {
    if (fileExt.compare(".html") == 0 || fileExt.compare(".htm") == 0) {
        return "text/html";
    } else if (fileExt.compare(".txt") == 0) {
        return "text/plain";
    } else if (fileExt.compare(".jpg") == 0 || fileExt.compare(".jpeg") == 0) {
        return "image/jpeg";
    } else if (fileExt.compare(".png") == 0) {
        return "image/png";
    } else {
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
			start = line.find("listen ") + 7;
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
			start = line.find("server_name ") + 12;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			if (wip != servName)
			{
				servName = wip;
			}
		}
		if (line.find("root") != std::string::npos)
		{
			start = line.find("root ") + 5;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			rootDir = wip;
		}
		if (line.find("error_page 404") != std::string::npos)
		{
			start = line.find("error_page 404 ") + 15;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			error404Dir = wip;
		}
	}
	return 1;
}

void Server::buildHTTPResponse(std::string fileName, std::string fileExt)
{
	std::string mimeType = getMIMEType(fileExt);
	std::cout << "file name = " << fileName << std::endl;
	std::string header;
	std::string buffer;
	std::stringstream headerStream;
	std::stringstream resonseStream;

	// if empty, aka front page
	if (fileName.empty())
	{
		resonseStream << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/plain\r\n" << "\r\n" << "Front page";
		response = resonseStream.str();
		return ;
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
			resonseStream << "HTTP/1.1 404 Not Found\r\n" << "Content-Type: text/plain\r\n" << "\r\n" << "Page you were looking for does not exist, nor should it ever exist";
			response = resonseStream.str();
			return ;
		}
		headerStream << "HTTP/1.1 404 Not Found\r\n" << "Content-Type: text/plain\r\n" << "\r\n";
		header = headerStream.str();
		response.append(header);
		std::getline(errormsg, buffer, '\0');
		response.append(buffer);
		return ;
	}
	headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Type: " << mimeType << "\r\n" << "\r\n";
	header = headerStream.str();
	std::cout << "is open" << std::endl;
	response.append(header);
	std::getline(file, buffer, '\0');
	response.append(buffer);
}

void Server::makeSocket(int port)
{
	socketList.push_back(listeningSocket(port));
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

// void Server::createPollfd(int numPorts)
// {
// 	fds = 
// 	for (int i = 0; i < numPorts; i++)
// 	{
// 		fds[i].fd = socketList.at(i).getServerFd();
// 		fds[i].events = POLLIN;
// 	}
// 	std::cout << "Server fds are :" << std::endl;
// 	for (int i = 0; i < numPorts; i++)
// 	{
// 		std::cout << fds[i].fd << std::endl;
// 	}
// }

struct pollfd Server::addToPollfd(struct pollfd fds[], int socketToAdd)
{
	struct pollfd temp[fdsSize + 1];
	std::cout << "here" << std::endl;
	for (int i = 0; i < fdsSize; i++)
	{
		temp[i] = fds[i];
	}
	fdsSize++;
	temp[fdsSize].fd = socketToAdd;
	temp[fdsSize].events = POLLIN;
	return *temp;
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
	// createPollfd(numPorts);
	struct pollfd fds[numPorts];
	fdsSize = numPorts;
	for (int i = 0; i < numPorts; i++)
	{
		fds[i].fd = socketList.at(i).getServerFd();
		fds[i].events = POLLIN;
	}
	std::cout << "Server fds are :" << std::endl;
	for (int i = 0; i < numPorts; i++)
	{
		std::cout << fds[i].fd << std::endl;
	}
	int pollResult;
	while (1)
	{
		// std::cout << "polling" << std::endl;
		pollResult = poll(fds, numPorts, 2000);
		if (pollResult == -1)
		{
			std::cout << "ERROR, " << strerror(errno) << std::endl;
			return ;
		}
		if (pollResult == 0)
			std::cout << "timeout" << std::endl;
		if (pollResult >= 1)
		{
			// std::cout << "got connection" << std::endl;
			int buffersize = 100;
			std::vector<char> buffer(buffersize);
			std::string received;
			long valread;
			for (int i = 0; i < numPorts; i++)
			{
				std::cout << "revents for " << i << " are " << fds[i].revents << std::endl;
			}
			
			for (int i = 0; i < fdsSize; i++)
			{
				if (fds[i].revents && POLLIN)
				{
					if (i < numPorts)
					{
						std::cout << "got connection on " << fds[i].fd << std::endl;
						struct sockaddr_in newAddress;
						newAddress = this->socketList.at(i).getAddress();
						int addrLen = sizeof(newAddress);
						int newSocket;
						int socketRes;
						socketRes = accept(this->socketList.at(i).getServerFd(), (struct sockaddr *)&newAddress, (socklen_t *)&addrLen);
						if (socketRes < 0)
						{
							std::cout << "ERROR, " << strerror(errno) << std::endl;
							return ;
						}
						addToPollfd(fds, newSocket);
						break ;
					}
					do
					{
						valread = recv(fds[i].fd, &buffer[0], buffer.size(), 0);
						// valread = recv(newSocket, &buffer[0], buffer.size(), 0);
						if (valread == -1)
							{
								std::cout << "ERROR, " << strerror(errno) << std::endl;
								return ;
							}
						else
						{
							received.append(buffer.cbegin(), buffer.cend());
						}
					} while (valread == buffersize);
					log(received);
					if (received.find("GET") != std::string::npos)
					{
						size_t start = received.find('/');
						size_t end = received.find(' ', start);
						std::string file = received.substr(start + 1, end - start - 1);
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
							// do CGI 
						}
						else
							buildHTTPResponse(fileName, fileExt);
						send(fds[i].fd, response.data(), response.length(), 0);
						// send(newSocket, response.data(), response.length(), 0);
						response.clear();
					}
					// close(newSocket);
				}
			}
			
		}
	}
	
	// struct sockaddr_in newAddress;
	// newAddress = this->socketList.at(0).getAddress();
	// std::cout << "port num = " << this->socketList.at(0).getPortNum() << std::endl;
	// int addrLen = sizeof(newAddress);
	// int newSocket;
	// while (1)
	// {
	// 	if ((newSocket = accept(this->socketList.at(0).getServerFd(), (struct sockaddr *)&newAddress, (socklen_t *)&addrLen)) < 0)
	// 	{
	// 		std::cout << "ERROR, " << strerror(errno) << std::endl;
	// 		return ;
	// 	}
	// 	std::vector<char> buffer(100);
	// 	std::string received;
	// 	long valread;
	// 	do
	// 	{
	// 		valread = recv(newSocket, &buffer[0], buffer.size(), 0);
	// 		if (valread == -1)
	// 			{
	// 				std::cout << "ERROR, " << strerror(errno) << std::endl;
	// 				return ;
	// 			}
	// 		else
	// 		{
	// 			received.append(buffer.cbegin(), buffer.cend());
	// 		}
	// 	} while (valread == 100);
	// 	log(received);
	// 	if (received.find("GET") != std::string::npos)
	// 	{
	// 		size_t start = received.find('/');
	// 		size_t end = received.find(' ', start);
	// 		std::string file = received.substr(start + 1, end - start - 1);
	// 		std::string fileExt = ".html";
	// 		std::string fileName;
	// 		if (file.find('.') != std::string::npos)
	// 		{
	// 			fileExt = file.substr(file.find('.'));
	// 			fileName = file.substr(0, file.find('.'));
	// 		}
	// 		else
	// 			fileName = file;
	// 		// std::cout << "name = " << fileName << " and ext = " << fileExt << std::endl;
	// 		if (fileExt.compare(".php") == 0)
	// 		{
	// 			// do CGI 
	// 		}
	// 		else
	// 			buildHTTPResponse(fileName, fileExt);
	// 		// std::cout << "response is :" << std::endl << response << std::endl;
	// 		send(newSocket, response.data(), response.length(), 0);
	// 		response.clear();
	// 	}
	// 	close(newSocket);
	// }
}