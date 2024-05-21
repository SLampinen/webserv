#include "incl/manager.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cout << "User ERROR, give a maximum of 1 config file" << std::endl;
		return 0;
	}
	Manager serverManager;
	if (argc == 2)
		serverManager.run(argv[1]);
	else
		serverManager.run(DEFAULTCONFIG);
	return 1;
}