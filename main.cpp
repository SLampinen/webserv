#include "incl/server.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cout << "User ERROR, give a maximum of 1 config file" << std::endl;
		return 0;
	}
	Server webserv;
	if (argc == 2)
		webserv.launch(argv[1]);
	else
		webserv.launch(DEFAULTCONFIG);
	return 1;
}