#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

#include "webserv.hpp"

class VirtualServer
{
	public:
		VirtualServer();
		~VirtualServer();

	void	init(std::istream& file);
	void	parseListen(std::istringstream& iss);
	void	parsePort(std::string& port);
	void	parseIpAddrs(void);

	int		&getPort();
	void	setPort(int port);
	int		&getFd();
	void	setfd(int fd);

	struct sockaddr_in sa;

	private:
		std::string	_ip;
		int			_port;
		
		std::string	_serverName;
		int			_socketfd;

};

#endif