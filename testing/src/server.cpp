#include "../incl/server.hpp"

Server::Server()
{
	this->servName = DEFAULTSERVNAME;
	this->port = DEFAULTPORT;
}

Server::~Server()
{
	std::cout << "Deleting server" << std::endl;
}

Server::Server(const Server &var)
{
	this->port = var.port;
	this->servName = var.servName;
	this->lSocket = var.lSocket;
}

Server &Server::operator=(const Server &var)
{
	if (this !=  &var)
	{
		this->port = var.port;
		this->servName = var.servName;
		this->lSocket = var.lSocket;
	}
	return (*this);
}

const char *Server::getMIMEType(const char *fileExt) {
    if (strcasecmp(fileExt, ".html") == 0 || strcasecmp(fileExt, ".htm") == 0) {
        return "text/html";
    } else if (strcasecmp(fileExt, ".txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(fileExt, ".jpg") == 0 || strcasecmp(fileExt, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(fileExt, ".png") == 0) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

void Server::readConfig(std::string fileName)
{
	std::fstream configFile;
	int start, end;
	std::string wip;

	configFile.open(fileName);
	if (!configFile.good())
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return ;
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
		// sets port if config file has one
		if (line.find("listen") != std::string::npos)
		{
			start = line.find("listen ") + 7;
			wip = line.substr(start);
			end = wip.find_first_not_of("0123456789");
			wip = wip.substr(0, end);
			if (!wip.empty())
				port = std::stoi(wip);
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
	}
}

void Server::buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen)
{
	const char *mimeType = getMIMEType(fileExt);
	char *header = (char *)malloc(BUFFERSIZE * sizeof(char));
	//if all works
	snprintf(header, BUFFERSIZE, 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"\r\n",
			mimeType);

	//if empty (front page)
	if (strcmp(fileName, "") == 0)
	{
		// std::cout << "empty" << std::endl;
		{
		snprintf(response, BUFFERSIZE, "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "Front page");
		*responseLen = strlen(response);
		}
		free(header);
		return ;
	}
	//if another page
	char *fileFull = (char *)malloc(sizeof(char) * (strlen(fileName) + strlen(fileExt) + 1));
	strcpy(fileFull, fileName);
	strcpy(fileFull + strlen(fileName), fileExt);
	int fileFd = open(fileFull, O_RDONLY);
	if (fileFd == -1)
	{
		snprintf(response, BUFFERSIZE, "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "Page you were looking for does not exist, nor should it ever exist");
		*responseLen = strlen(response);
		free(header);
		free(fileFull);
		return ;
	}
	*responseLen = 0;
	memcpy(response, header, strlen(header));
	*responseLen = strlen(header);
	ssize_t bytesRead;
	while ((bytesRead = read(fileFd, response + *responseLen, BUFFERSIZE - *responseLen)) > 0)
	{
		*responseLen += bytesRead;
	}
	free(fileFull);
	free(header);
}

void Server::makeSocket()
{
	if (this->lSocket.getPortNum() != port)
	{
		this->lSocket = listeningSocket(port);
	}
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
	logfile << "New entry in log, at time : " << timeBuffer << std::endl;
	logfile << text;
	logfile << std::endl << std::endl;
	logfile.close();
}

void Server::launch(std::string configFile)
{
	readConfig(configFile);
	std::cout << "server name = " << servName << std::endl;
	makeSocket();
	struct sockaddr_in newAddress;
	newAddress = this->lSocket.getAddress();
	int addrLen = sizeof(newAddress);

	int newSocket;
	while (1)
	{
		if ((newSocket = accept(this->lSocket.getServerFd(), (struct sockaddr *)&newAddress, (socklen_t *)&addrLen)) < 0)
		{
			std::cout << "ERROR, " << strerror(errno) << std::endl;
			return ;
		}
		std::vector<char> buffer(100);
		std::string received;
		long valread;
		do
		{
			valread = recv(newSocket, &buffer[0], buffer.size(), 0);
			if (valread == -1)
				{
					std::cout << "ERROR, " << strerror(errno) << std::endl;
					return ;
				}
			else
			{
				received.append(buffer.cbegin(), buffer.cend());
			}
		} while (valread == 100);
		// std::cout << received << std::endl;
		log(received);
	if (received.find("GET") != std::string::npos)
	{
		std::cout << "getting " << std::endl;
		size_t start = received.find('/');
		size_t end = received.find(' ', start);
		std::cout << "e - s = " << end - start << std::endl;
		std::string file = received.substr(start + 1, end - start - 1);
		std::cout << file << std::endl;
		std::string fileExt = "html";
		std::string fileName;
		if (file.find('.') != std::string::npos)
		{
			fileExt = file.substr(file.find('.'));
			fileName = file.substr(0, file.find('.'));
		}
		else
			fileName = file;
		std::cout << "name = " << fileName << " and ext = " << fileExt << std::endl;
		char *response = (char *)malloc(BUFFERSIZE * sizeof(char));
		size_t responseLen;
		buildHTTPResponse(fileName.data(), fileExt.data(), response, &responseLen);
		send(newSocket, response, responseLen, 0);
	}
		close(newSocket);
	}
}