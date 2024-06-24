#include "manager.hpp"
#include <filesystem> // ! debug

int main(int argc, char **argv)
{
	std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
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