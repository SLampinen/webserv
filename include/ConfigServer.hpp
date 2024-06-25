#ifndef WS_CONFIGSERVER_HPP
# define WS_CONFIGSERVER_HPP

#include <cstring>
#include <vector>
#include <map>
#include "ConfigSection.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"

class ConfigServer : public ConfigSection {
	public:
		ConfigServer();
		~ConfigServer() {}

		void initialize();
		bool addConfigServerName(const std::string name);
		bool addErrorPage(const std::string nbr, const std::string file_path);
		void addLocation(Location location);

		// accessed after setup
		bool matchRequest(const Request &request);
		bool matchPort(const size_t port);
		bool resolveLocation(int const method, std::string const &request_path, size_t &index);
		Response resolveRequest(int const method, std::string const &request_path);
		Response resolveRequest(const Request &request);
		std::string getErrorPage(const size_t page_num);

		// added to make compatible with existing Server
		std::string getName() const;
		size_t getSize() const;
		size_t getNumOfPorts() const;
		int getPort(size_t index);
		bool isThereLocationMatch();
		Location &getMatchedLocation();

	private:
		size_t _max_client_body_size;
		std::vector<size_t> _ports;
		std::vector<std::string> _server_names;
		std::map<size_t, std::string> _error_pages;
		std::vector<Location> _locations;
		size_t _last_matched_location;
	public:
		class ConfigServerException : public std::exception {
			public:
				ConfigServerException(std::string const msg) : _msg(msg) {}
				const char *what() const noexcept override { return _msg.c_str(); }
			private:
			const std::string _msg;
		};
	public: //debug
		void printData();
		std::string const printId() const;
};

#endif