#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"

# define DEFAULTCONFIG "incl/config.conf"
# define DEFAULT404DIR "error/404Default.html"
# define POLL_TIMEOUT 1000

class Server
{
private:
	int numOfPorts;
	std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
	std::map<int, std::string> errorPages;
	std::string cgiExt;
	std::string cgiPath;
	size_t client_max_body_size;
	bool directoryIndex;
public:
	Server();
	~Server();
	Server(const Server &var);
	Server& operator=(const Server &var);

	std::string makeStatus(int status);

	std::string makeStatus2xx(int status);
	std::string makeStatus3xx(int status);
	std::string makeStatus4xx(int status);
	std::string makeStatus5xx(int status);
	std::string makeHeader(int responseStatus, int responseSize);
	std::string buildHTTPResponse(std::string fileName, std::string fileExt);

	//Not used at the moment
	std::string getMIMEType(std::string fileExt);

	std::string getServerName(void);
	std::string getCGIExt(void);
	std::string getCGIPath(void);
	int getNumOfPorts(void);
	size_t getClientBodySize(void);
	std::string getRootDir(void);

	void makeSocketList(void);

	void addPort(int port);
	void setServerName(std::string name);
	void setRootDir(std::string dir);
	void setErrorDir(std::string dir);
	void setCGIExt(std::string ext);
	void setCGIPath(std::string path);
	void setClientBodySize(std::string size);

	void log(std::string text);
	void print(void);

	std::vector<listeningSocket> listeners;

};

#endif
