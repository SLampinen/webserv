#include "server.hpp"


Server::~Server() {} // std::cout << "Deleting server" << std::endl; }

Server::Server(ConfigServer &cfg_srv, ConfigSection &def_res) : csrv(cfg_srv), def_res(def_res) { 
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

// ! merge version by rleskine TODO: update getErrorPage in ConfigServer
std::string Server::makeHeader(int responseStatus, int responseSize) {
	std::cout << "makeHeader called with " << std::to_string(responseStatus) << " " << std::to_string(responseSize) << std::endl;
	std::string header = "HTTP/1.1 " + std::to_string(responseStatus) + " ";
	std::string error_page = csrv.getErrorPage(responseStatus);
	if (!error_page.empty()) {
		std::ifstream error_file(error_page);
		if (!error_file)
			error_page = "Opening error file [" + error_page + "] failed!";
		else {
			std::getline(error_file, error_page, '\0');
			error_file.close();
		}
	}
	// def_res.printAll();
	// std::cout << "RES_STS_STR: " << std::to_string(responseStatus) << std::endl;
	// std::cout << "DEF_RES404: " << def_res.getIndexArg("404", 0) << std::endl;
	// std::cout << "DEF_RES404: " << def_res.getIndexArg("404", 1) << std::endl;
	//std::cout << "def_restest: " << def_re
	header += def_res.getIndexArg(std::to_string(responseStatus), 1);
	if (responseSize == 0 && !error_page.empty())
		responseSize = error_page.size();
	header += "\r\nContent-Length: " + std::to_string(responseSize) + "\r\n\r\n";
	if (!error_page.empty())
		header += error_page;
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
		//fileFull.append("home.html");
		fileFull.append(indexFile);
		std::cout << "the front page is " << fileFull << std::endl;
		open(fileFull.data(), O_RDONLY);
		std::ifstream file(fileFull);
		if (file.is_open() == 0)
		{
			return makeHeader(404, 0);
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
			//std::string body = "Page you were looking for does not exist, nor should it ever exist";
			response = makeHeader(404, 0);
			//response.append(body);
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