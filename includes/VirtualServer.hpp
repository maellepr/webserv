#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

#include "webserv.hpp"
#include "Location.hpp"

#define KB_IN_BYTES 1024;
#define MB_IN_BYTES 1048576;
#define	GB_IN_BYTES 1073741824; 

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
	int		parseErrorCode(std::string& code);
	// void	parsePathErrorPage(std::string& path);
	void	parseIndex(std::istringstream& iss);

	int								&getPort();
	void							setPort(int port);
	int								&getFd();
	void							setfd(int fd);
	size_t							getMaxBodySize();
	std::string						&getIP();
	std::string						&getServerName();
	bool							&getDefaultVS();
	bool							&getPortByDefault();
	std::map<std::string, Location>	&getLocations();

	void	connectVirtualServers();


	private:
		struct sockaddr_in _address;
	
		std::string	_ip;
		int			_port;
		std::string	_rootDir;
		std::string	_autoIndex;
		bool		_indexOnOff;
		size_t		_maxBodySize;// in bytes by default
		std::map<int, std::string>	_errorPages;
		std::vector<std::string>	_indexPages;

		std::vector<std::string>	_serverNames;
		std::string					_serverName;  // GARDER CELUI DE MAELLE

		std::map<std::string, Location> _location;// 
		int			_socketfd;

		bool		_defaultVS; // GARDER CELUI DE MAELLE
		bool		_portByDefault; // Indicates if the port was set by default 8080 or if it was defined in the config file

};

#endif