
#include "../incl/webserv.hpp"
#define PORT 4242

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

const char *getMIMEType(const char *fileExt) {
    if (strcasecmp(fileExt, "html") == 0 || strcasecmp(fileExt, "htm") == 0) {
        return "text/html";
    } else if (strcasecmp(fileExt, "txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(fileExt, "jpg") == 0 || strcasecmp(fileExt, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(fileExt, "png") == 0) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

void buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen)
{
	const char *mimeType = getMIMEType(fileExt);
	char *header = (char *)malloc(1000 * sizeof(char));
	snprintf(header, 1000, 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"\r\n",
			mimeType);

	if (strcmp(fileName, "") == 0)
	{
		// std::cout << "empty" << std::endl;
		{
		snprintf(response, 1000, "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "Starting page, only file.txt exists");
		*responseLen = strlen(response);
		}
		free(header);
		return ;
	}

	int fileFd = open(fileName, O_RDONLY);
	if (fileFd == -1)
	{
		snprintf(response, 1000, "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "Page you were looking for does not exist, nor should it ever exist");
		*responseLen = strlen(response);
		free(header);
		return ;
	}
	*responseLen = 0;
	memcpy(response, header, strlen(header));
	*responseLen = strlen(header);
	ssize_t bytesRead;
	while ((bytesRead = read(fileFd, response + *responseLen, 1000 - *responseLen)) > 0)
	{
		*responseLen += bytesRead;
	}
	free(header);
}

void handleClient(int clientFd)
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

		handleClient(newSocket);
		// write(newSocket, hello.data(), hello.size());
		// std::cout << "Sent hello message" << std::endl;
		close(newSocket);
	}
	return 0;
}