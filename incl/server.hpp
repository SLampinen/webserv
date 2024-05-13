#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define BUFFERSIZE 10000
# define DEFAULTCONFIG "incl/config.conf"

class Server
{
private:
	int port;
	std::string servName;
	std::string rootDir;
	std::string response;
	// size_t responseLen;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);
	listeningSocket lSocket;
	void launch(std::string configFile);
	std::string getMIMEType(std::string fileExt);
	void readConfig(std::string fileName);
	void makeSocket();
	// void buildHTTPResponse(const char *fileName, const char *fileExt, char *response, size_t *responseLen);
	void buildHTTPResponse(std::string fileName, std::string fileExt);
	void log(std::string text);
};

#endif
