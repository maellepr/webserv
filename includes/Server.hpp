#ifndef SERVER_HPP
# define SERVER_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"
#include "Client.hpp"

class Server
{
	public:
		Server();
		~Server();
		
		bool	init(const char *filename);
		void	loop();
		// void	connectVirtualServers();

	private:
		std::vector<VirtualServer>	_virtualServers;
		std::map<int, Client>		_clients;
		fd_set 		_all_sockets;
		fd_set		_read_fds;
		fd_set		_write_fds;
		int			_fd_max;

		int	_acceptNewConnection(int server_socket);


};


#endif