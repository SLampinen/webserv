#ifndef MANAGER_HPP
# define MANAGER_HPP
# include "library.hpp"
# include "socket.hpp"
# include "server.hpp"

class Manager
{
private:
	int numOfServers;
	std::vector<Server> serverList;
	std::vector<std::pair<int, int> > serverIndex;
public:
	Manager();
	~Manager();
	Manager(const Manager &var);
	Manager& operator=(const Manager &var);
	void run(std::string configFile);
	int readConfig(std::string fileName);
};

#endif