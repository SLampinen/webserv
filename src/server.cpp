#include "../incl/server.hpp"

Server::Server()
{
	this->listener = listeningSocket(DEFAULTPORT);
	this->servName = "defaultserv";
	this->port = DEFAULTPORT;
	this->error404Dir = DEFAULT404DIR;
	this->cgiExt = "";
	this->cgiPath = "";
	this->client_max_body_size = 0;
	this->numOfPorts = 0;
	this->ports.push_back(DEFAULTPORT);
}

Server::~Server()
{
	std::cout << "Deleting server" << std::endl;
}

Server::Server(const Server &var)
{
	this->port = var.port;
	this->listener = var.listener;
	this->servName = var.servName;
	this->rootDir = var.rootDir;
	this->error404Dir = var.error404Dir;
	this->cgiExt = var.cgiExt;
	this->cgiPath = var.cgiPath;
	this->client_max_body_size = var.client_max_body_size;
	this->numOfPorts = var.numOfPorts;
	this->ports = var.ports;
	this->listeners = var.listeners;
}

Server &Server::operator=(const Server &var)
{
	if (this != &var)
	{
		this->port = var.port;
		this->listener = var.listener;
		this->servName = var.servName;
		this->rootDir = var.rootDir;
		this->error404Dir = var.error404Dir;
		this->cgiExt = var.cgiExt;
		this->cgiPath = var.cgiPath;
		this->client_max_body_size = var.client_max_body_size;
		this->numOfPorts = var.numOfPorts;
		this->ports = var.ports;
		this->listeners = var.listeners;
	}
	return (*this);
}

void Server::print(void)
{
	std::cout << "server name = " << servName << std::endl;
	std::cout << "(relative) root dir = " << rootDir << std::endl;
	std::cout << "error dir = " << error404Dir << std::endl;
	std::cout << "cgi path and ext  = " << cgiPath << " and " << cgiExt << std::endl;
	std::cout << "Num of ports = " << numOfPorts << std::endl;
	std::cout << "ports are " << std::endl;
	for (int i = 0; i < numOfPorts; i++)
	{
		std::cout << ports.at(i) << std::endl;
	}
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

void Server::setClientBodySize(std::string size)
{
	this->client_max_body_size = std::stoi(size);
}

int Server::getClientBodySize(void)
{
	return this->client_max_body_size;
}

std::string Server::getCGIPath(void)
{
	return this->cgiPath;
}

std::string Server::getCGIExt(void)
{
	return this->cgiExt;
}

std::string Server::getMIMEType(std::string fileExt)
{
	if (fileExt.compare(".html") == 0 || fileExt.compare(".htm") == 0)
	{
		return "text/html";
	}
	else if (fileExt.compare(".txt") == 0)
	{
		return "text/plain";
	}
	else if (fileExt.compare(".jpg") == 0 || fileExt.compare(".jpeg") == 0)
	{
		return "image/jpeg";
	}
	else if (fileExt.compare(".png") == 0)
	{
		return "image/png";
	}
	else
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
		headerStream << "HTTP/1.1 200 OK\r\n"
					 << "Content-Length: " << buffer.size() << "\r\n"
					 << "\r\n";
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
			responseStream << "HTTP/1.1 404 Not Found\r\n"
						   << "Content-Length: 66\r\n"
						   << "\r\n"
						   << "Page you were looking for does not exist, nor should it ever exist";
			response = responseStream.str();
			return response;
		}
		std::getline(errormsg, buffer, '\0');
		headerStream << "HTTP/1.1 404 Not Found\r\n"
					 << "Content-Length: " << buffer.size() << "\r\n"
					 << "\r\n";
		header = headerStream.str();
		response.append(header);
		response.append(buffer);
		return response;
	}
	// headerStream << "HTTP/1.1 200 OK\r\n" << "Content-Type: " << mimeType << "\r\n" << "\r\n";
	std::getline(file, buffer, '\0');
	headerStream << "HTTP/1.1 200 OK\r\n"
				 << "Content-Length: " << buffer.size() << "\r\n"
				 << "\r\n";
	header = headerStream.str();
	response.append(header);
	response.append(buffer);
	return response;
}

void setnonblocking(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Server::makeSocket(int portNum)
{
	listeningSocket newSocket(portNum);
	setnonblocking(newSocket.getServerFd()); // Set the listening socket to non-blocking mode
	this->listener = newSocket;				 // Add the new socket to the list
	std::cout << "Socket for port " << portNum << " created and added to the list." << std::endl;
}

void Server::log(std::string text)
{
	std::ofstream logfile;
	logfile.open("logfile.txt", std::ofstream::app);
	if (logfile.is_open() == 0)
	{
		std::cout << "Failed to open logfile.txt" << std::endl;
		return;
	}
	time_t rawtime;
	struct tm *timeinfo;
	char timeBuffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeBuffer, 80, "%T %d:%m:%Y", timeinfo);
	logfile << "----------------------------------------------------------------------------------------------------" << std::endl;
	logfile << "New entry in log, at time " << timeBuffer << ":" << std::endl;
	logfile << text;
	logfile << std::endl
			<< std::endl;
	logfile.close();
}

void Server::addPort(int port)
{
	std::cout << "HERE port = " << port << std::endl;
	int i;
	for (i = 0; i < numOfPorts; i++)
	{
		if (ports.at(i) == port)
			break;
	}
	std::cout << "HERE, i = " << i << " numOfports = " << numOfPorts << std::endl;
	if (i == numOfPorts)
	{
		if (i == 0)
			ports.at(i) = port;
		else
			ports.push_back(port);
		numOfPorts++;
	}
}

int Server::getNumOfPorts(void)
{
	return this->numOfPorts;
}

int Server::getNthPort(int n)
{
	return this->ports.at(n);
}

void Server::makeSocketList()
{
	for (int i = 0; i < numOfPorts; i++)
	{
		listeningSocket newSocket(ports.at(i));
		setnonblocking(newSocket.getServerFd());
		listeners.push_back(newSocket);
	}
}
