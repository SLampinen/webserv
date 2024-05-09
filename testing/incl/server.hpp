#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define BUFFERSIZE 1000

class Server
{
private:
	int port;
public:
	Server(/* args */);
	~Server();
	listeningSocket lSocket;
	void launch(std::string configFile);
	const char *getMIMEType(const char *fileExt);
	void readConfig(std::string fileName);
	void makeSocket();
	void buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen);
};

#endif