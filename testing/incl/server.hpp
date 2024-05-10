#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define BUFFERSIZE 10000

class Server
{
private:
	int port;
	std::string servName;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);
	listeningSocket lSocket;
	void launch(std::string configFile);
	const char *getMIMEType(const char *fileExt);
	void readConfig(std::string fileName);
	void makeSocket();
	void buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen);
	void log(std::string text);
};

#endif