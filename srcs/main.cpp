#include "../includes/webserv.hpp"
#include "../includes/Server.hpp"

bool noSignal = true;
std::map<StatusCode, std::string>	STATUS_MESSAGES;
std::map<std::string, std::string>	CONTENT_TYPES;
std::map<unsigned char, int>		HEXA_BASE;

int main(int argc, char **argv)
{
	// if (argc != 2)
	// {
	// 	std::cout << "Error" << std::endl << "One argument expected : configuration file." << std::endl;
	// 	return 1;
	// }

	Server server;
	try 
	{
		std::signal(SIGINT, handleSigint);
		signal(SIGPIPE, &handleSigpipe);
		if (argc != 2)
			server.init("conf/valid/goodconf.conf");
		else
			server.init(argv[1]);
		server.loop();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;		
		return 1;
	}
	return 0;
}
