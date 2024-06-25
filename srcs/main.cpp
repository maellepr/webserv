#include "../includes/webserv.hpp"
#include "../includes/Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Error" << std::endl << "One argument expected : configuration file." << std::endl;
		return 1;
	}

	Server server;
	try 
	{
		if (!server.init(argv[1]))
			return 1;
			server.loop();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}