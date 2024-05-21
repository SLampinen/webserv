#ifndef MANAGER_HPP
# define MANAGER_HPP
# include "library.hpp"
# include "socket.hpp"
# include "server.hpp"

class Manager
{
private:
	int numOfServers;
	std::string servName;
	std::string rootDir;
	std::string error404Dir;
	std::string cgiExt;
	std::string cgiPath;
	std::vector<Server> serverList;

public:
	Manager();
	~Manager();
	Manager(const Manager &var);
	Manager& operator=(const Manager &var);
	void run(std::string configFile);
	int readConfig(std::string fileName);

	std::vector <listeningSocket> socketList;
};

#endif