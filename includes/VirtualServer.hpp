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



		int								&getPort();
		void							setPort(int port);

		int								&getFd();
		void							setfd(int fd);

		size_t							getMaxBodySize();

		std::string						&getIP();
		void							setIp(std::string ip);

		bool							&getPortByDefault();
		std::map<std::string, Location>	&getLocations();

		int		&getIsBind();
		void	setIsBind(int bind);

		std::string	&getServerName();

		bool	&getToErase();
		void	setToErase(bool erase);

		bool	&getDefaultVS();
		void	setDefaultVS(bool value);

		int		&getSocketFd();

		void	setIndex(int i);
		int		&getIndex();

		void	init(std::istream& file);
		void	connectVirtualServers();

	private:
		struct sockaddr_in _address;
	
		int			_index;			// FOR PARSING num du server dans l'ordre
		std::string	_ip;			// listen -> ip_address (ex: 127.0.0.2)
		std::string	_ipParse;		// FOR PARSING
		int			_port;			// listen -> port (ex: 8080)
		std::string	_rootDir;		// root -> configurer tel que "www" pour l'instant

		std::string	_autoIndex;		// FOR PARSING
		bool		_indexOnOff;	// autoindex -> si true = on, si false = non renseigne ou off
		size_t		_maxBodySize;	// client_max_body_size -> in bytes by default, max 3M
		std::map<int, std::string>	_errorPages;	// error_page -> map avec int = error code, std::string = page correspondante 
		std::map<int, std::string>	_returnPages;	// return -> map avec int = error code, std::string = page correspondante
		std::vector<std::string>	_indexPages;	// index -> vector avec les pages

		// std::vector<std::string>	_serverNames;
		std::string	_serverName;	// server_name

		std::map<std::string, Location> _location;	// location -> map avec string = prefix, Location = un obj Location 

		int		_isBind;			// FOR PARSING 0 not bind but should be bind / 1 bind

		int		_socketfd;			// 
		bool	_ipByDefault;		// true = ip non renseignee donc 0.0.0.0, false = ip renseignee
		bool	_portByDefault;		// true = port non renseigne donc 8080, false = port renseigne
		bool	_toErase;			// FOR PARSING

		bool	_defaultVS;			// true = default_server ecrit dans le conf file ou 1er dans la list, false = contraire

		

		void	parseListen(std::istringstream& iss);
		void	parsePort(std::string& port);
		void	parseIpAddrs(void);
		void	parseServerNames(std::istringstream& iss);
		void	parseRoot(std::istringstream& iss);
		void	parseAutoIndex(std::istringstream& iss);
		void	parseMaxClientBodySize(std::istringstream& iss);
		void	parseErrorPages(std::istringstream& iss);
		int		parseErrorCode(std::string& code);
		void	parseReturn(std::istringstream& iss);
		int		parseCodeReturn(std::string& code);
		// void	parsePathErrorPage(std::string& path);
		void	parseIndex(std::istringstream& iss);
		void	parseDefaultServer(std::istringstream& iss);

};

#endif