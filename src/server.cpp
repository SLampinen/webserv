#include "server.hpp"

// Server::Server() {
// 	this->servName = "defaultserv";
// 	this->error404Dir = DEFAULT404DIR;
// 	this->cgiExt = "";
// 	this->cgiPath = "";
// 	this->client_max_body_size = 0;
// 	this->numOfPorts = 0;
// 	this->directoryIndex = true;
// }

Server::~Server() { std::cout << "Deleting server" << std::endl; }

// Server::Server(const Server &var)
// {
// 	this->servName = var.servName;
// 	this->rootDir = var.rootDir;
// 	this->error404Dir = var.error404Dir;
// 	this->errorPages = var.errorPages;
// 	this->cgiExt = var.cgiExt;
// 	this->cgiPath = var.cgiPath;
// 	this->client_max_body_size = var.client_max_body_size;
// 	this->numOfPorts = var.numOfPorts;
// 	this->ports = var.ports;
// 	this->listeners = var.listeners;
// 	this->directoryIndex = var.directoryIndex;
// }

Server::Server(ConfigServer &cfg_srv) : csrv(cfg_srv) { 
	servName = cfg_srv.getName();
	rootDir = "";
	cgiExt = "";
	cgiPath = "";
	client_max_body_size = cfg_srv.getSize();
	numOfPorts = cfg_srv.getNumOfPorts();
}

void Server::setLocation(Location &loc) {
	rootDir = loc.getRootPath();
	//if (rootDir.back() == '/') rootDir = rootDir.substr(0, rootDir.size() - 1);
	cgiExt = loc.getLastCGISuffix();
	cgiPath = loc.getLastCGIPath();
	directoryIndex = loc.directoryIndexAllowed();
	indexFile = loc.defaultIndexFile();
}

std::string Server::getServerName(void) { return this->servName; }
size_t Server::getClientBodySize(void) { return this->client_max_body_size; }
std::string Server::getCGIPath(void) { return this->cgiPath; }
std::string Server::getCGIExt(void) { return this->cgiExt; }

std::string Server::getMIMEType(std::string fileExt)
{
	if (fileExt.compare(".html") == 0 || fileExt.compare(".htm") == 0) { return "text/html"; }
	else if (fileExt.compare(".txt") == 0) { return "text/plain"; }
	else if (fileExt.compare(".jpg") == 0 || fileExt.compare(".jpeg") == 0) { return "image/jpeg"; }
	else if (fileExt.compare(".png") == 0) { return "image/png"; }
	else { return "application/octet-stream"; }
}

std::string Server::makeStatus(int status)
{
	for (std::map<int, std::string>::iterator iter = errorPages.begin(); iter != errorPages.end(); iter++)
	{
		if (status == iter->first)
		{
			std::string errorDir = rootDir;
			errorDir.append("error/");
			errorDir.append(iter->second);
			std::ifstream errormsg(errorDir);
			std::string buffer;
			std::getline(errormsg, buffer, '\0');
			return buffer;
		}
	}
	return "ERROR";
}

std::string Server::makeStatus2xx(int status)
{
	if (status == 200)
		return " OK";

	if (status == 201)
		return " Created";

	if (status == 202)
		return " Accepted";

	if (status == 203)
		return " Non-Authoritative Information";

	return " ERROR";
}

std::string Server::makeStatus3xx(int status)
{
	if (status == 300)
		return " Multiple Choices";

	return " ERROR";
}

std::string Server::makeStatus4xx(int status)
{
	if (status == 400)
		return " Bad Request";

	if (status == 403)
	{
		return " Forbidden";
	}

	if (status == 404)
		return " Not Found";

	if (status == 408)
	{
		return " Connection timeout";
	}

	if (status == 413)
		return " Request Entity Too Large";

	if (status == 418)
		return " I'm a teapot";

	return " ERROR";
}

std::string Server::makeStatus5xx(int status)
{
	if (status == 500)
		return " Internal Server Error";

	if (status == 501)
		return " Method Not Implemented";

	return " ERROR";
}
// std::string Server::makeHeader(int responseStatus, int responseSize)
// {
// 	std::stringstream headerStream;
// 	std::string header;
// 	headerStream << "HTTP/1.1 " << responseStatus;
// 	if (responseStatus >= 100 && responseStatus <= 199)
// 		headerStream << " ";
// 	else if (responseStatus >= 200 && responseStatus <= 299)
// 		headerStream << makeStatus2xx(responseStatus) << "\r\n";
// 	else if (responseStatus >= 300 && responseStatus <= 399)
// 		headerStream << makeStatus3xx(responseStatus) << "\r\n";
// 	else if (responseStatus >= 400 && responseStatus <= 499)
// 		headerStream << makeStatus4xx(responseStatus) << "\r\n";
// 	else if (responseStatus >= 500 && responseStatus <= 599)
// 		headerStream << makeStatus5xx(responseStatus) << "\r\n";
// 	//not used right now, will be used once config parser gets updated
// 	std::stringstream newStream;
// 	newStream << makeStatus(responseStatus) << "\r\n";
// 	std::cout << "DEBUG: " << newStream.str() << std::endl;
// 	//end of future/debug
// 	headerStream << "Content-Length: " << responseSize << "\r\n\r\n";
// 	header = headerStream.str();
// 	if (header.find("ERROR") != std::string::npos)
// 	{
// 		std::cout << "Created ERROR, this is bad" << std::endl;
// 		return "ERROR";
// 	}
// 	return header;
// }

// ! merge version by rleskine TODO: csrv missing, update getErrorPage in ConfigServer
std::string Server::makeHeader(int responseStatus, int responseSize) {
	std::string header = "HTTP/1.1 " + std::to_string(responseStatus) + " ";
	//std::string error_page;
	std::string error_page = csrv.getErrorPage(responseStatus);
	if (!error_page.empty())
		header += csrv.getErrorPage(responseStatus); // ! update getErrorpage to fetch file
	else
		header += "PUT DEFAULT RESPONSE TEXT HERE";
	header += "\r\nContent-Length: " + std::to_string(responseSize) + "\r\n\r\n";
	return header;
}

std::string Server::buildHTTPResponse(std::string fileName, std::string fileExt)
{
	std::cout << "BUILDING" << std::endl;
	std::string response;
	std::string mimeType = getMIMEType(fileExt);
	std::cout << "file name = " << fileName << std::endl;
	std::string header;
	std::string buffer;
	std::stringstream responseStream;

	// if empty, aka front page
	if (fileName.empty())
	{
		std::string fileFull;
		fileFull.append(rootDir);
		fileFull.append("home.html");
		std::cout << "the front page is " << fileFull << std::endl;
		open(fileFull.data(), O_RDONLY);
		std::ifstream file(fileFull);
		if (file.is_open() == 0)
		{
			responseStream << "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nFront page";
			response = responseStream.str();
			return response;
		}
		std::getline(file, buffer, '\0');
		header = makeHeader(200, buffer.size());
		response.append(header);
		response.append(buffer);
		return response;
	}

	std::string possibleDir = rootDir;
	possibleDir.append(fileName);
	DIR *dir = opendir(possibleDir.c_str());
	if (dir != NULL)
	{
		if (directoryIndex == false)
		{
			closedir(dir);
			buffer = "You are trying to access a directory, not file.\nThis is not allowed";
			header = makeHeader(403, buffer.size());
			response.append(header);
			response.append(buffer);
			return response;
		}
		else
		{
			struct dirent *pDirent;
			std::stringstream bufferStream;
			pDirent = readdir(dir);
			bufferStream << "The directory contains files: \n";
			while (pDirent != NULL)
			{
				bufferStream << pDirent->d_name << std::endl;
				std::cout << pDirent->d_name << std::endl;
				pDirent = readdir(dir);
			}
			closedir(dir);
			header = makeHeader(200, bufferStream.str().size());
			response.append(header);
			response.append(bufferStream.str());
			return response;
		}
	}

	// if some other page
	std::string fileFull;
	fileFull.append(rootDir);
	fileFull.append(fileName);
	fileFull.append(fileExt);
	std::cout << fileFull << std::endl;
	open(fileFull.data(), O_RDONLY);
	std::ifstream file(fileFull);
	if (file.is_open() == 0)
	{
		std::string errorDir;
		errorDir.append(rootDir);
		errorDir.append(error404Dir);
		std::ifstream errormsg(errorDir);
		if (errormsg.is_open() == 0)
		{
			std::string body = "Page you were looking for does not exist, nor should it ever exist";
			response = makeHeader(404, body.size());
			response.append(body);
			return response;
		}
		std::getline(errormsg, buffer, '\0');
		header = makeHeader(404, buffer.size());
		response.append(header);
		response.append(buffer);
		return response;
	}
	std::getline(file, buffer, '\0');
	header = makeHeader(200, buffer.size());
	response.append(header);
	response.append(buffer);
	return response;
}

void setnonblocking(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
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
	strftime(timeBuffer, 80, "%T %d.%m.%Y", timeinfo);
	logfile << "----------------------------------------------------------------------------------------------------" << std::endl;
	logfile << "New entry in log, at time " << timeBuffer << std::endl;
	logfile << text;
	logfile << std::endl
			<< std::endl;
	logfile.close();
}

// void addPort(int port)
// {
// 	std::cout << "HERE port = " << port << std::endl;
// 	int i;
// 	for (i = 0; i < numOfPorts; i++)
// 	{
// 		if (ports.at(i) == port)
// 			break;
// 	}
// 	std::cout << "HERE, i = " << i << " numOfports = " << numOfPorts << std::endl;
// 	if (i == numOfPorts)
// 	{
// 		ports.push_back(port);
// 		numOfPorts++;
// 	}
// }

int Server::getNumOfPorts(void) { return csrv.getNumOfPorts(); }

void Server::makeSocketList()
{
	for (size_t i = 0; i < csrv.getNumOfPorts(); i++)
	{
		listeningSocket newSocket(csrv.getPort(i));
		setnonblocking(newSocket.getServerFd());
		listeners.push_back(newSocket);
	}
}

std::string Server::getRootDir()
{
	 return this->rootDir;
}

// Server &Server::operator=(const Server &var)
// {
// 	if (this != &var)
// 	{
// 		this->servName = var.servName;
// 		this->rootDir = var.rootDir;
// 		this->error404Dir = var.error404Dir;
// 		this->errorPages = var.errorPages;
// 		this->cgiExt = var.cgiExt;
// 		this->cgiPath = var.cgiPath;
// 		this->client_max_body_size = var.client_max_body_size;
// 		this->numOfPorts = var.numOfPorts;
// 		this->ports = var.ports;
// 		this->listeners = var.listeners;
// 		this->directoryIndex = var.directoryIndex;
// 	}
// 	return (*this);
// }

void Server::print(void)
{
	std::cout << "server name = " << servName << std::endl;
	std::cout << "(relative) root dir = " << rootDir << std::endl;
	std::cout << "error dir = " << error404Dir << std::endl;
	std::cout << "cgi path and ext  = " << cgiPath << " and " << cgiExt << std::endl;
	std::cout << "Num of ports = " << csrv.getNumOfPorts() << std::endl;
	std::cout << "ports are " << std::endl;
	for (size_t i = 0; i < csrv.getNumOfPorts(); i++)
	{
		std::cout << csrv.getPort(i) << std::endl;
	}
}