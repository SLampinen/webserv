#include "manager.hpp"

void readDefaultResponses(ConfigSection &def_res) {
	std::fstream file("cfg/default_responses");
	std::string line;
	if (!file) throw std::logic_error("Couldn't open default responses file!");
	if (!std::getline(file, line) || line != "# INTERNAL FILE, DO NOT MODIFY") 
		throw std::logic_error("Invalid default responses file!");
	while (std::getline(file, line)) {
		std::vector<std::string> vec;
		vec.push_back("Default Responses");
		vec.push_back(line.substr(0, line.find(" ")));
		vec.push_back(line.substr(line.find(" "), std::string::npos));
		def_res.addConfigLine(vec);
	}
}

// Read config
int Manager::readConfig(ConfigParser &config_parser)
{
	readDefaultResponses(default_responses);
	if (!config_parser.startParse())
		return (std::cerr << "Failed to parse configuration file" << std::endl, 0);
	while (!config_parser.endParse()) {
		ConfigServer config_server(config_parser.getServer());
		for (ConfigServer &existing_server : configserverList) {
			for (size_t index = 0; index < config_server.getNumOfPorts(); index++)
				if (existing_server.matchPort(config_server.getPort(index)))
					return (std::cerr << "Duplicate ports specified in different servers!" << std::endl, 0);
		}
		configserverList.push_back(config_server);
		serverList.push_back(Server(configserverList.back(), default_responses));
		serverList.back().makeSocketList();
	}
	return 1;
}