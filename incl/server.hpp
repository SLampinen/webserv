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
	int port;
	int numOfPorts;
	std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
	std::string cgiExt;
	std::string cgiPath;
	int client_max_body_size;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);

	std::string makeStatus2xx(int status);
	std::string makeStatus3xx(int status);
	std::string makeStatus4xx(int status);
	std::string makeStatus5xx(int status);
	std::string makeHeader(int responseStatus, int responseSize);
	std::string buildHTTPResponse(std::string fileName, std::string fileExt);

	std::string getMIMEType(std::string fileExt);

	std::string getServerName(void);
	int getPort(void);
	int getNthPort(int n);
	std::string getCGIExt(void);
	std::string getCGIPath(void);
	int getNumOfPorts(void);
	int getClientBodySize(void);
	std::string getRootDir(void);

	void makeSocket(int portNum);
	void makeSocketList(void);
	void addPort(int port);

	void setPort(int portNum);
	void setServerName(std::string name);
	void setRootDir(std::string dir);
	void setErrorDir(std::string dir);
	void setCGIExt(std::string ext);
	void setCGIPath(std::string path);
	void setClientBodySize(std::string size);

	void log(std::string text);
	void print(void);

	listeningSocket listener;
	std::vector<listeningSocket> listeners;
};

#endif
