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
	void	parseServerNames(std::istringstream& iss);
	void	parseRoot(std::istringstream& iss);
	void	parseAutoIndex(std::istringstream& iss);
	void	parseMaxClientBodySize(std::istringstream& iss);
	void	parseErrorPages(std::istringstream& iss);

	int		&getPort();
	void	setPort(int port);
	int		&getFd();
	void	setfd(int fd);

	void	connectVirtualServers();


	private:
		struct sockaddr_in _address;
	
		std::string	_ip;
		int			_port;
		std::string	_rootDir;
		std::string	_index;
		bool		_indexOnOff;
		size_t		_maxBodySize;
		std::map<int, std::string>	_errorPages;
		std::vector<std::string>	_indexPages;

		std::vector<std::string>	_serverNames;
		int			_socketfd;

		// size_t	_maxBodySize;

};

#endif