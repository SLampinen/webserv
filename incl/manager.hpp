#ifndef MANAGER_HPP
# define MANAGER_HPP
# include "library.hpp"
# include "socket.hpp"
# include "server.hpp"

// Connection timeout must be greater than response timeout
// lest great horrors emerge
# define RESPONSE_TIMEOUT 60
# define CONNECTION_TIMEOUT 120

class Manager
{
private:
	
	std::vector<Server> serverList;
	std::vector<std::pair<int, int> > serverIndex;

	std::vector<std::pair<int, int> > pids;

	std::vector<struct pollfd> fds;
	std::vector<int> fdsTimestamps;
	std::vector<int> cgiOnGoing;

	// for testing and debugging purposes
	// we may not need this in full project

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
	void handleUpload(std::string receivedData, std::string boundary, std::vector <struct pollfd> fds, int i);
};

#endif