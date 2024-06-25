#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

#include "webserv.hpp"

class VirtualServer
{
	public:
		VirtualServer();
		~VirtualServer();

	bool	init(void);

	int		&getPort();
	int		&getFd();
	void	setfd(int fd);

	private:
		int			_port;
		std::string	_serverName;
		int			_socketfd;

};

#endif