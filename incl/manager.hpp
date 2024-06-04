#ifndef MANAGER_HPP
# define MANAGER_HPP
# include "library.hpp"
# include "socket.hpp"
# include "server.hpp"

class Manager
{
private:
	int numOfServers;
	int managerPid;
	std::vector<Server> serverList;
	std::vector<std::pair<int, int> > serverIndex;
	std::vector<struct pollfd> fds;
	std::vector<int> cgiOnGoing;
	std::vector<std::pair<int, int> > timers;
	// we may not need this in full project
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
	void handleOther(std::string receivedData, std::vector <struct pollfd> fds, int i);

	void handleCGI(std::string receivedData, std::vector <struct pollfd> fds, int i);
};

#endif