#include "manager.hpp"

// Read config
int Manager::readConfig(std::string config_file)
{
	//std::fstream configFile;
	//int start, end;
	//std::string wip;

	std::cout << "opening config file: " << config_file << std::endl;
	ConfigParser config_parser(config_file);

	if (!config_parser.startParse())
		return 0;
	std::vector<ConfigServer> configServerList;
	while (!config_parser.endParse()) {
		configServerList.push_back(config_parser.getServer());
		serverList.push_back(Server(configServerList.back()));
	}
	return 1;



	// configFile.open(fileName);
	// if (!configFile.good())
	// {
	// 	std::cout << "ERROR, " << strerror(errno) << std::endl;
	// 	return 0;
	// }
	// std::string line;
	// while (1)
	// {
	// 	std::getline(configFile, line);
	// 	if (configFile.eof())
	// 		break;
	// 	if (line.find("#") != std::string::npos)
	// 	{
	// 		end = line.find("#");
	// 		line = line.substr(0, end);
	// 	}
	// 	if (line.find("server {") != std::string::npos)
	// 	{
	// 		std::cout << "----------start of server block----------" << std::endl;
	// 		std::string serverName = "server.num";
	// 		serverName.append(std::to_string(serverList.size()));
	// 		Server newServer;
	// 		std::cout << serverName << std::endl;
	// 		std::cout << "The (default) name of server is " << newServer.getServerName() << std::endl;
	// 		while (1)
	// 		{
	// 			std::getline(configFile, line);
	// 			std::cout << "line in block is : " << line << std::endl;
	// 			if (configFile.eof())
	// 				break;
	// 			if (line.find("#") != std::string::npos)
	// 			{
	// 				end = line.find("#");
	// 				line = line.substr(0, end);
	// 			}
	// 			if (line.find("}") != std::string::npos)
	// 			{
	// 				std::cout << "----------end of block----------" << std::endl;
	// 				break;
	// 			}
	// 			if (line.find("server_name") != std::string::npos)
	// 			{
	// 				start = line.find("server_name") + 12;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setServerName(wip);
	// 			}
	// 			if (line.find("listen") != std::string::npos)
	// 			{
	// 				start = line.find("listen") + 7;
	// 				wip = line.substr(start);
	// 				end = wip.find_first_not_of("0123456789");
	// 				wip = wip.substr(0, end);
	// 				if (!wip.empty())
	// 				{
	// 					std::cout << "setting port" << std::endl;
	// 					newServer.addPort(std::stoi(wip));
	// 				}
	// 			}
	// 			if (line.find("root") != std::string::npos)
	// 			{
	// 				start = line.find("root") + 5;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setRootDir(wip);
	// 			}
	// 			if (line.find("error_page 404") != std::string::npos)
	// 			{
	// 				start = line.find("error_page 404") + 15;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setErrorDir(wip);
	// 			}
	// 			if (line.find("cgi_ext") != std::string::npos)
	// 			{
	// 				start = line.find("cgi_ext") + 8;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setCGIExt(wip);
	// 			}
	// 			if (line.find("cgi_path") != std::string::npos)
	// 			{
	// 				start = line.find("cgi_path") + 9;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setCGIPath(wip);
	// 			}
	// 			if (line.find("client_max_body_size") != std::string::npos)
	// 			{
	// 				start = line.find("client_max_body_size") + 21;
	// 				wip = line.substr(start);
	// 				end = wip.find(";");
	// 				wip = wip.substr(0, end);
	// 				newServer.setClientBodySize(wip);
	// 			}
	// 		}
	// 		std::cout << "end of server block" << std::endl;
	// 		if (newServer.getNumOfPorts() > 0)
	// 		{
	// 			newServer.makeSocketList();
	// 			serverList.push_back(newServer);
	// 		}
	// 		else
	// 		{
	// 			std::cout << "ERROR: Server has 0 ports" << std::endl;
	// 		}
	// 	}
	// }
	// if (serverList.size() == 0)
	// {
	// 	std::cout << "ERROR: Config file has no servers" << std::endl;
	// 	return 0;
	// }
	// return 1;
}