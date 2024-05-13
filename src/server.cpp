#include "../incl/server.hpp"

Server::Server()
{
	this->numPorts = 0;
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
	// this->port = var.port;
	this->servName = var.servName;
	this->lSocket = var.lSocket;
	this->response = var.response;
	this->error404Dir = var.error404Dir;
}

Server &Server::operator=(const Server &var)
{
	if (this !=  &var)
	{
		this->numPorts = var.numPorts;
		this->ports = var.ports;
		// this->port = var.port;
		this->servName = var.servName;
		this->lSocket = var.lSocket;
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
		// sets port if config file has one
		if (line.find("listen") != std::string::npos)
		{
			start = line.find("listen ") + 7;
			wip = line.substr(start);
			end = wip.find_first_not_of("0123456789");
			wip = wip.substr(0, end);
			if (!wip.empty())
			{
				int i;
				// port = std::stoi(wip);
				for (i = 0; i < numPorts; i++)
				{
					if (ports.at(i) == std::stoi(wip))
						break ;
				}
				if (i == numPorts)
				{
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

//todo: change this
void Server::makeSocket(int port)
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
	logfile << "New entry in log, at time " << timeBuffer << ":" << std::endl;
	logfile << text;
	logfile << std::endl << std::endl;
	logfile.close();
}

// TODO: change accept to poll( or equivalent)

void Server::launch(std::string configFile)
{
	if (readConfig(configFile) == 0)
		return ;
	std::cout << "server name = " << servName << std::endl;
	std::cout << "num of ports = " << numPorts << std::endl;
	for (int i = 0; i < numPorts; i++)
	{
		makeSocket(ports.at(i));
	}
	
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
		// std::cout << "name = " << fileName << " and ext = " << fileExt << std::endl;
		if (fileExt.compare(".php") == 0)
		{
			// do CGI 
		}
		else
			buildHTTPResponse(fileName, fileExt);
		// std::cout << "response is :" << std::endl << response << std::endl;
		send(newSocket, response.data(), response.length(), 0);
		response.clear();
	}
		close(newSocket);
	}
}