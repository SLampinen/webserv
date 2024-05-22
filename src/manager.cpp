#include "../incl/manager.hpp"

Manager::Manager()
{
	numOfServers = 0;
}

Manager::~Manager()
{

}

Manager::Manager(const Manager &var)
{
	this->numOfServers = var.numOfServers;
}

Manager &Manager::operator=(const Manager &var)
{
	if (this != &var)
	{
		this->numOfServers = var.numOfServers;
	}
	return (*this);
}

void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Manager::run(std::string configFile)
{
	if (readConfig(configFile) == 0)
	{
		std::cout << "ERROR reading config file" << std::endl;
		return ;
	}
	std::cout << "number of servers = " << numOfServers << std::endl;
	for (int i = 0; i < numOfServers; i++)
	{
		serverList.at(i).makeSockets();
		std::cout << "Sockets are made for " << i << std::endl;
	}
	for (int i = 0; i < numOfServers; i++)
	{
		// serverList.at(i).launch();
		serverList.at(i).print();
	}
	
}

int Manager::readConfig(std::string fileName)
{

	std::fstream configFile;
	int start, end;
	std::string wip;

	configFile.open(fileName);
	if (!configFile.good())
	{
		std::cout << "ERROR, " << strerror(errno) << std::endl;
		return 0;
	}
	std::string line;
	while (1)
	{
		std::getline(configFile, line);
		std::cout << line << std::endl;
		if (!line.compare("\0") || configFile.eof())
			break;
		//remove comments
		if (line.find("#") != std::string::npos)
		{
			end = line.find("#");
			line = line.substr(0, end);
		}
		if (line.find("server {") != std::string::npos)
		{
			std::cout << "start of server block" << std::endl;
			std::string serverName = "server.num";
			serverName.append(std::to_string(numOfServers));
			Server newServer;
			std::cout << serverName << std::endl;
			std::cout << "The (default) name of server is " << newServer.getServerName() << std::endl;
			numOfServers++;
			while (1)
			{
				std::getline(configFile, line);
				// std::cout << line << std::endl;
				if (!line.compare("\0") || configFile.eof())
					break;
				if (line.find("#") != std::string::npos)
				{
					end = line.find("#");
					line = line.substr(0, end);
				}
				if (line.find("}") != std::string::npos)
				{
					std::cout << "----------end of block----------" << std::endl;
					break;
				}
				if (line.find("server_name") != std::string::npos)
				{
					start = line.find("server_name") + 12;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setServerName(wip);
				}
				if (line.find("listen") != std::string::npos)
				{
					start = line.find("listen") + 7;
					wip = line.substr(start);
					end = wip.find_first_not_of("0123456789");
					wip = wip.substr(0, end);
					if (!wip.empty())
					{
						newServer.addPort(std::stoi(wip));
					}
				}
				if (line.find("root") != std::string::npos)
				{
					start = line.find("root") + 5;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setRootDir(wip);
				}
				if (line.find("error_page 404") != std::string::npos)
				{
					start = line.find("error_page 404") + 15;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setErrorDir(wip);
				}
				if (line.find("cgi_ext") != std::string::npos)
				{
					start = line.find("cgi_ext") + 8;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setCGIExt(wip);
				}
				if (line.find("cgi_path") != std::string::npos)
				{
					start = line.find("cgi_path") + 9;
					wip = line.substr(start);
					end = wip.find(";");
					wip = wip.substr(0, end);
					newServer.setCGIPath(wip);
				}
				
			}
			newServer.print();
			serverList.push_back(newServer);
			std::cout << "end of server block" <<std::endl;
		}
	}
	return 1;
}
