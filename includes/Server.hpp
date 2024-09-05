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
		
		void	init(const char *filename);
		void	loop();
		// void	connectVirtualServers();
		

	private:
		std::vector<VirtualServer>					_virtualServers;
		std::map<int, Client>						_clients;
		std::map<int, std::vector<VirtualServer*> >	_socketBoundVs;
		fd_set 										_all_sockets;
		fd_set										_read_fds;
		fd_set										_write_fds;
		int											_fd_max;
		std::vector<int>							_socketMax;
		std::map<int, int>							_maxConnections;

		int											_nbServers; // for parsing Maelle

		void	_eraseVSIfDuplicate(std::vector<VirtualServer> &VirtualServersTemp);
		void	_prepareVSToConnect();
		int		_acceptNewConnection(int server_socket);
		void	_ipIsAnyAddress(size_t i);
		void	_ipIsSpecificAddress(size_t i);

		void	_checkDuplicateDefaultServer();
		void	_defineVSByDefault(std::map<int, std::vector<VirtualServer*> >::iterator it);

		// void	_checkNecessaryServerName();

		void	_addBindedVS(size_t i, std::vector<VirtualServer*> &bindedVS);

};

#endif