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
	ConfigSection &def_res;

public:
	~Server();
	Server(ConfigServer &cfg_server, ConfigSection &def_res);

	void setLocation(Location &loc);
	std::string makeStatus(int status);
	std::string makeHeader(int responseStatus, int responseSize);
	std::string buildHTTPResponse(std::string fileName, std::string fileExt);

	std::string getMIMEType(std::string fileExt);

	std::string getServerName(void);
	std::string getCGIExt(void);
	std::string getCGIPath(void);
	int getNumOfPorts(void);
	size_t getClientBodySize(void);
	std::string getRootDir(void);
	bool dirIndexAllowed();

	void makeSocketList(void);

	void log(std::string text);

	std::vector<listeningSocket> listeners;

};

#endif
