#include "webserv.hpp"
#include "VirtualServer.hpp"

#ifndef SERVER_HPP
# define SERVER_HPP

class Server
{
	public:
		Server();
		~Server();
		
		bool	init(const char *filename);
		void	loop();
		void	connectVirtualServers();

	private:
		std::vector<VirtualServer>	_virtualServers;
};

#endif