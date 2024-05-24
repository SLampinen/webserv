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
	std::vector<std::pair<int, int> > connectionTime;
	std::vector<std::string> data;
public:
	Manager();
	~Manager();
	Manager(const Manager &var);
	Manager& operator=(const Manager &var);
	void run(std::string configFile);
	int readConfig(std::string fileName);
	void handleGet(std::string receivedData, std::vector <struct pollfd> fds, int i);
	void handlePost(std::string receivedData, std::vector <struct pollfd> fds, int i);
	void handleDelete(std::string receivedData, std::vector <struct pollfd> fds, int i);
};

#endif