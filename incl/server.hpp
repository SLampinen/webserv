#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define DEFAULTCONFIG "incl/config.conf"
# define DEFAULT404DIR "error/404Default.html"
# define CONNECTION_TIMEOUT 5
# define POLL_TIMEOUT 1000

class Server
{
private:
	int numPorts;
	int fdsSize;
	std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
	std::string cgiExt;
	std::string cgiPath;
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
