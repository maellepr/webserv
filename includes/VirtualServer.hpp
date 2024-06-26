#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

#include "webserv.hpp"

class VirtualServer
{
	public:
		VirtualServer();
		~VirtualServer();

	bool	init(std::istream& file);
	bool	parseListen(std::istringstream& iss);

	int		&getPort();
	void	setPort(int port);
	int		&getFd();
	void	setfd(int fd);

	private:
		int			_port;
		std::string	_serverName;
		int			_socketfd;

};

#endif