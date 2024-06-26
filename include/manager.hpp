#ifndef MANAGER_HPP
#define MANAGER_HPP
#include "library.hpp"
#include "socket.hpp"
#include "server.hpp"
#include "ConfigParser.hpp"

// Connection timeout must be greater than response timeout
// lest great horrors emerge
#define REQUEST_TIMEOUT 3
#define RESPONSE_TIMEOUT 60
#define CONNECTION_TIMEOUT 120

class Manager
{
private:

	// struct FileTransferState
	// {
	// 	std::unique_ptr<std::ofstream> file;
	// 	bool transferInProgress;
	// };
	// std::map<int, FileTransferState> clientStates;
	std::vector<Server> serverList;
	std::vector<ConfigServer> configserverList;
	ConfigSection default_responses;

	std::vector<std::pair<int, size_t>> serverIndex;
	std::vector<std::pair<int, size_t>> pids;

	std::vector<struct pollfd> fds;
	std::vector<int> fdsTimestamps;
	std::vector<int> cgiOnGoing;
	std::vector<std::string> magic;//Does nothing, but removing it breaks the code
	std::vector<std::pair<std::string, std::string> > boundaries;
	std::vector<std::pair<int, std::string> > fdsFileNames;
public:
	Manager();
	~Manager();
	//Manager(const Manager &var);
	//Manager &operator=(const Manager &var);

	void setupPollingforServers();

	void handlePolling();

	bool acceptNewConnections(size_t index);

	void handleClientCommunication(size_t index);

	void handleTimeout(size_t index, int k);

	void handleWorkDone(size_t index, int k);

	void handleCgiWork(size_t index);

	void handlePollEvent(size_t index);

    bool checkReceive(int bytesReceived, size_t index);

    void closeConnections(size_t index, std::string message);

    void closeInactiveConnections(size_t index);

    void run(std::string configFile);

	int readConfig(ConfigParser &config_parser);

	Server &prepareServer(int const method, std::string file_path, std::vector<struct pollfd> fds, int i, Response &response);
	bool prepareFailure(int code, std::vector<struct pollfd> fds, int i);
	void handleGet(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handlePost(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handleDelete(std::string receivedData, std::vector<struct pollfd> fds, int i);
	void handleOther(std::string receivedData, std::vector<struct pollfd> fds, int i);

	void handleCGI(std::string receivedData, std::vector<struct pollfd> fds, int i);

	void handleUpload(std::string receivedData, std::string boundary, std::vector<struct pollfd> fds, int i);
	void handleChunk(std::string receivedData, std::vector<struct pollfd> fds, int fdsIndex, int boundariesIndex);
	void handleContinue(std::string receivedData, int fdsIndex);
	void handleTimeout(int fdsIndex);
};

#endif