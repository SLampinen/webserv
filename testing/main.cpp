#include "incl/server.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "User ERROR, give a config file" << std::endl;
		return 0;
	}
	Server webserv;
	webserv.launch(argv[1]);

}