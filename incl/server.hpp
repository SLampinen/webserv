#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define DEFAULTCONFIG "incl/config.conf"
# define DEFAULT404DIR "error/404Default.html"

class Server
{
private:
	int numPorts;
	int fdsSize;
	std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);
	std::vector <listeningSocket> socketList;
	void launch(std::string configFile);
	std::string getMIMEType(std::string fileExt);
	int readConfig(std::string fileName);
	void makeSocket(int port);
	std::string buildHTTPResponse(std::string fileName, std::string fileExt);
	void log(std::string text);
};

#endif
