#ifndef MANAGER_HPP
#define MANAGER_HPP
#include "library.hpp"
#include "socket.hpp"
#include "server.hpp"

// Connection timeout must be greater than response timeout
// lest great horrors emerge
#define RESPONSE_TIMEOUT 60
#define CONNECTION_TIMEOUT 120

class Manager
{
private:
	std::vector<Server> serverList;
	std::vector<std::pair<int, int>> serverIndex;

	std::vector<std::pair<int, int>> pids;

	std::vector<struct pollfd> fds;
	std::vector<int> fdsTimestamps;
	std::vector<int> cgiOnGoing;
	std::vector<std::string> magic;//Does nothing, but removing it breaks the code
	std::vector<std::pair<std::string, std::string> > boundaries;
public:
	Manager();
	~Manager();
	Manager(const Manager &var);
	Manager &operator=(const Manager &var);

	void setupPollingforServers();

	void handlePolling();

	bool acceptNewConnections(size_t index);

	void handleClientCommunication(size_t index);

	void handleTimeout(size_t index, int k);

	void handleWorkDone(size_t index, int k);

	void handleCgiWork(size_t index);

	void handlePollEvent(size_t index);

	void closeInactiveConnections(size_t index);

	void run(std::string configFile);

	int readConfig(std::string fileName);

	void handleGet(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handlePost(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handleDelete(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handleOther(std::string receivedData, std::vector<struct pollfd> fds, int i);

	void handleCGI(std::string receivedData, std::vector<struct pollfd> fds, int i);

	void handleUpload(std::string receivedData, std::string boundary, std::vector<struct pollfd> fds, int i);

	bool isLastChunk(const std::string& data);
};

#endif