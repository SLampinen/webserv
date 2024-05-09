#include "../incl/server.hpp"

Server::Server()
{
	
}

Server::~Server()
{
	std::cout << "Deleting server" << std::endl;
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
	std::string name;

	configFile.open(fileName);
	if (!configFile.good())
	{
		std::cout << "ERROR, open file failed" << std::endl;
		return ;
	}
	std::string line;
	while(1)
	{
		std::getline(configFile, line);
		// std::cout << line << std::endl;
		if (!line.compare("\0") || configFile.eof())
			break;
		if (line.find("listen") != std::string::npos)
		{
			start = line.find("listen ") + 7;
			wip = line.substr(start);
			end = wip.find_first_not_of("0123456789");
			wip = wip.substr(0, end);
			port = std::stoi(wip);
			std::cout << "port = " << port << std::endl;
		}
		if (line.find("server_name") != std::string::npos)
		{
			start = line.find("server_name ") + 12;
			wip = line.substr(start);
			end = wip.find(";");
			wip = wip.substr(0, end);
			name = wip;
		}
	}

}

void Server::buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen)
{
	const char *mimeType = getMIMEType(fileExt);
	char *header = (char *)malloc(BUFFERSIZE * sizeof(char));
	// all works
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

void Server::launch(std::string configFile)
{
	readConfig(configFile);
	makeSocket();
	struct sockaddr_in newAddress;
	newAddress = this->lSocket.getAddress();
	int addrLen = sizeof(newAddress);

	int newSocket;
	while (1)
	{
		if ((newSocket = accept(this->lSocket.getServerFd(), (struct sockaddr *)&newAddress, (socklen_t *)&addrLen)) < 0)
		{
			std::cout << "ERROR, accpet failed" << std::endl;
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
					std::cout << "ERROR with receving data" << std::endl;
					return ;
				}
			else
			{
				received.append(buffer.cbegin(), buffer.cend());
			}
		} while (valread == 100);
		std::cout << received << std::endl;
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