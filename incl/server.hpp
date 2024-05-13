#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define BUFFERSIZE 10000
# define DEFAULTCONFIG "incl/config.conf"
# define DEFAULT404DIR "error/404Default.html"

class Server
{
private:
	// int port;
	int numPorts;
	std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string response;
	std::string error404Dir;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);
	// change away from single socket to multiple
	listeningSocket lSocket;
	void launch(std::string configFile);
	std::string getMIMEType(std::string fileExt);
	int readConfig(std::string fileName);
	void makeSocket(int port);
	void buildHTTPResponse(std::string fileName, std::string fileExt);
	void log(std::string text);
};

#endif
