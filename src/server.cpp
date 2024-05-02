
#include "../incl/webserv.hpp"
#define PORT 4242
#define BUFFER_SIZE 104857600 //100 megabytes

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

const char *getMIMEType(const char *fileExt) {
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

void buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen)
{
	// std::cout << "at bhttpr file name = " << fileName << " and fileExt = " << fileExt << std::endl;
	const char *mimeType = getMIMEType(fileExt);
	char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
	snprintf(header, BUFFER_SIZE, 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"\r\n",
			mimeType);

	// std::cout << strcmp(fileName, "") << std::endl;
	if (strcmp(fileName, "") == 0)
	{
		// std::cout << "empty" << std::endl;
		{
		snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "Starting page, only file.txt exists");
		*responseLen = strlen(response);
		}
		free(header);
		return ;
	}
	char *fileFull = (char *)malloc(sizeof(char) * (strlen(fileName) + strlen(fileExt) + 1));
	strcpy(fileFull, fileName);
	strcpy(fileFull + strlen(fileName), fileExt);
	// std::cout << "full name of file = " << fileFull << std::endl;
	int fileFd = open(fileFull, O_RDONLY);
	if (fileFd == -1)
	{
		snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\n"
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
	while ((bytesRead = read(fileFd, response + *responseLen, BUFFER_SIZE - *responseLen)) > 0)
	{
		*responseLen += bytesRead;
	}
	free(fileFull);
	free(header);
}

/*void handleClient(int clientFd)
{
	char buffer[10000] = {0};
	long valread;
	valread = recv(clientFd, buffer, sizeof(buffer), 0);
		std::cout << buffer << std::endl;
	if (strncmp(buffer, "GET", 3) == 0)
	{
		// std::cout << "get something" << std::endl;
		int start, end;
		for (start = 0; ; start++)
		{
			if (buffer[start] == '/')
			{
				start++;
				break;
			}
		}
		for (end = start; ; end++)
		{
			if (buffer[end] == ' ' || buffer[end] == '\r')
			{
				break;
			}
		}
		// std::cout << "start = " << start << " , end = " << end << std::endl;
		// std::cout << buffer[start] << " and " << buffer[end] << std::endl;
		char *fileName = (char *)malloc(1000 * sizeof(char));
		strncpy(fileName, buffer + start, end - start);
		// std::cout << fileName << std::endl;
		char *response = (char *)malloc(1000 * sizeof(char));
		size_t responseLen;
		buildHTTPResponse(fileName , "txt", response, &responseLen);
		send(clientFd, response, responseLen, 0);
		free(response);
	}
}*/

void handleClient(int clientFd)
{
	std::vector<char> buffer(100);
	std::string received;
	long valread;
	do
	{
		valread = recv(clientFd, &buffer[0], buffer.size(), 0);
		if (valread == -1)
			error("ERROR with receving data");
		else
		{
			received.append(buffer.cbegin(), buffer.cend());
		}
	} while (valread == 100);
	// std::cout << received << std::endl;
	if (received.find("GET") != std::string::npos)
	{
		size_t start = received.find('/');
		size_t end = received.find(' ', start);
		// std::cout << "e - s = " << end - start << std::endl;
		std::string file = received.substr(start + 1, end - start - 1);
		// std::cout << file << std::endl;
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
		

		//testing c++
		// std::vector<char> response;
		// size_t responseLen;
		// buildHTTPResponse(fileName.data(), fileExt.data(), response, &responseLen);

		//working c code
		char *response = (char *)malloc(BUFFER_SIZE * sizeof(char));
		size_t responseLen;
		buildHTTPResponse(fileName.data(), fileExt.data(), response, &responseLen);



		send(clientFd, response, responseLen, 0);
		// free(response);
	}
}

int main(int argc, char **argv)
{
	int serverFd;
	int newSocket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		error("ERROR, socket failed");
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	//To avoid socket already in use error
	const int enable = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		error("ERROR, bind failed");
	}

	if (listen(serverFd, 10) < 0)
	{
		error("ERROR, listen failed");
	}

	// std::string hello = "Hello world\n";
	while (1)
	{
		std::cout << "Waiting for connection" << std::endl;

		if ((newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t *)&addrlen)) < 0)
		{
			error("ERROR, accept failed");
		}
		// std::cout << "from port " << ntohs(address.sin_port) << std::endl;
		handleClient(newSocket);
		// write(newSocket, hello.data(), hello.size());
		// std::cout << "Sent hello message" << std::endl;
		close(newSocket);
	}
	return 0;
}