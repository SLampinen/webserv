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
	void launch();
	std::string getMIMEType(std::string fileExt);
	std::string buildHTTPResponse(std::string fileName, std::string fileExt);
	std::string getServerName(void);

	int getPort(void);

	void makeSocket(int port);

	void setPort(int portNum);
	void setServerName(std::string name);
	void setRootDir(std::string dir);
	void setErrorDir(std::string dir);
	void setCGIExt(std::string ext);
	void setCGIPath(std::string path);

	void log(std::string text);
	void print(void);

	listeningSocket socketList;
};

#endif
