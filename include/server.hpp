#ifndef SERVER_HPP
# define SERVER_HPP
# include "library.hpp"
# include "socket.hpp"
# include "ConfigServer.hpp"

# define DEFAULTCONFIG "cfg/config.conf"
# define DEFAULT404DIR "error/404Default.html"
# define POLL_TIMEOUT 1000

class Server {
private:
	int numOfPorts;
	//std::vector<int> ports;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
	std::map<int, std::string> errorPages;
	std::string cgiExt;
	std::string cgiPath;
	size_t client_max_body_size;
	bool directoryIndex;
	std::string indexFile;
	ConfigServer csrv;

	//Server(const Server &var);
public:
	//Server();
	~Server();
	Server(ConfigServer &cfg_server);
	//Server& operator=(const Server &var);

	void setLocation(Location &loc); // changes values to match the current requests location // ! added by rleskine

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
	bool methodAllowed(size_t method); // 1GET 2POST 3DEL

	void makeSocketList(void);

	void log(std::string text);
	void print(void);

	std::vector<listeningSocket> listeners;

};

#endif
