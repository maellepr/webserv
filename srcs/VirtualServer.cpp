#include "../includes/VirtualServer.hpp"

VirtualServer::VirtualServer()
{
	memset(&_address, 0, sizeof _address);
	_address.sin_family = AF_INET; // IPv4			
	_address.sin_port = htons(8080);// port par defaut
}

VirtualServer::~VirtualServer()
{
}

void	VirtualServer::init(std::istream& file)
{
	// _port = 8080;
	// _serverName = "www.minigoats.com";
	std::string	line, keyword;
	bool	empty = true;
	while (std::getline(file, line))
	{
		std::cerr << "line vs = " << line << "\n";
		std::istringstream iss(line);

		// iss >> keyword;

		if (!(iss >> keyword) || keyword[0] == '#')
			continue ;
		std::cerr << "keyword vs = " << keyword << "\n";
		if (keyword == "}" && empty == true)
			throw ErrorConfigFile("Error in the config file : empty server section");
		else if (keyword == "}" && empty == false)
			return ;
		else if (keyword == "listen")
		{
			parseListen(iss);
		}
		else if (keyword == "server_name")
		{
			parseServerNames(iss);
		}
		else if (keyword == "root")
		{
			parseRoot(iss);
		}
		else if (keyword == "autoindex")
		{
			parseAutoIndex(iss);
		}
		else if (keyword == "client_max_body_size")
		{
			parseMaxClientBodySize(iss);
		}
		else if (keyword == "error_page")
		{
			parseErrorPages(iss);
		}
		else if (keyword == "index")
		{
			continue ;
		}
		else if (keyword == "location")
		{
			continue ;
		}
		else
			throw ErrorConfigFile("Error in the config file : wrong keyword");
		empty = false;
	}
	return ;
}

void	VirtualServer::parseListen(std::istringstream& iss)
{
	std::string	line;
	// int	port;
	if (!(iss >> line))
		throw ErrorConfigFile("Error in the config file : listen : missing information");
	if (line.find_first_not_of("0123456789.:") != std::string::npos)
		throw ErrorConfigFile("Error in the config file : listen : wrong content");
	// std::cerr << "line pL = " << line << "\n";
	size_t	index = line.find(':');
	if (index != std::string::npos)// : est a ete trouve dans line
	{
		_ip = line.substr(0, index);
		parseIpAddrs();
		// std::cerr << "ipAddrs = " << ipAddrs << "\n"; 
		std::string	port = line.substr(index + 1, line.size());
		parsePort(port);
		std::cerr << "_port = " << _port << "\n";
	}
	else
	{
		std::string	port = line;
		parsePort(port);
		std::cerr << "_port else = " << _port << "\n";
	}
	if (iss >> line)
		throw ErrorConfigFile("Error in the config file : listen : wrong content");
}

void	VirtualServer::parsePort(std::string& port)
{
	if (port.empty() || port.find_first_not_of("0123456789") != std::string::npos || port.size() > 5)
		throw ErrorConfigFile("Error in the conf file : listen : wrong port");
	std::stringstream ss(port);
	ss >> _port;
	if (_port > 65535)// valeur max port
		throw ErrorConfigFile("Error in the conf file : listen : wrong port");
}

void	VirtualServer::parseIpAddrs(void)
{
	if (_ip.empty()/* || _ipAddrs.find_first_not_of("0123456789.") != std::string::npos*/)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
	if (_ip == "localhost")
	{
		_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		return ;
	}
	if (_ip == "*")
	{
		_address.sin_addr.s_addr = htonl(INADDR_ANY);
		return ;
	}
	if (_ip.find_first_not_of("0123456789.") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
	std::cerr << "ipAddrs = " << _ip << "\n";
	int	dots = 0;
	for (int i = 0; _ip[i]; i++)
	{	
		if (_ip[i] == '.')
		{
			_ip.replace(i, 1, 1, ' ');
			dots++;
		}
	}
	if (dots != 3)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
	std::istringstream iss(_ip);
	std::string	more;
	int	ip[4];
	if (!(iss >> ip[0]) || !(iss >> ip[1]) || !(iss >> ip[2]) || !(iss >> ip[3]))
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
	int res = 0;
	for (int i = 0; i < 4; i++)
	{
		if (ip[i] < 0 || ip[i] > 255)
			throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
		res = res << 8 | ip[i];
	}
	if (iss >> more)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
	std::cerr << "res ipAddrs = " << res << "\n";
	_address.sin_addr.s_addr = htonl(res);
}

void	VirtualServer::parseServerNames(std::istringstream& iss)
{
	std::string	serverName;
	if (!(iss >> serverName))
		throw ErrorConfigFile("Error in the conf file : no server name");
	_serverNames.push_back(serverName);
	while (iss >> serverName)
		_serverNames.push_back(serverName);
}

void	VirtualServer::parseRoot(std::istringstream& iss)
{
	std::string	path;
	
	if (!(iss >> path))
		throw ErrorConfigFile("Error in the conf file : no root");
	std::cerr << "path = " << path << "\n";
	if (path.compare(0, 8, "/var/www") == 0)
		_rootDir = "www";
	else
		throw ErrorConfigFile("Error in the conf file : root : wrong content");
	// _rootDir = path + (path[path.size() - 1] == '/' ? "" : "/");
	// check if line ends with '/' -> if it does add nothing
	//							   -> if it doesn't add '/'
	if (iss >> path)
		throw ErrorConfigFile("Error in the conf file : root : wrong content");
	if (_rootDir.empty())
		throw ErrorConfigFile("Error in the conf file : root : wrong content");
	std::cerr << "_rootdir = " << _rootDir << "\n"; 
	
    struct stat info;

    if (stat(_rootDir.c_str(), &info) != 0)// cannot access path
		throw ErrorConfigFile("Error : root : cannot access path or file");
    // if (S_ISDIR(info.st_mode) != 0)// is not a directory
	// 	throw ErrorConfigFile("Error : root : the path is not a directory");
}

void	VirtualServer::parseAutoIndex(std::istringstream& iss)
{
	if (!(iss >> _index) || _index.empty())
		throw ErrorConfigFile("Error in the conf file : auto index not specified");
	if (_index == "on")
		_indexOnOff = true;
	else if (_index == "off")
		_indexOnOff = false;
}

void	VirtualServer::parseMaxClientBodySize(std::istringstream& iss)
{
	std::string	maxClientBS;

	if (!(iss >> maxClientBS) || maxClientBS.empty())
		throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong content");
	size_t	index = maxClientBS.find_first_not_of("0123456789"); 
	if (index == 0)
		throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong content");
	_maxBodySize = std::strtol(maxClientBS.c_str(), NULL, 10);
	if (maxClientBS[index] != '\0')
	{
		if (maxClientBS[index + 1] == '\0')
		{
			if (maxClientBS[index] == 'k' || maxClientBS[index] == 'K')
			{
				_maxBodySize = _maxBodySize << 10;
			}
			else if (maxClientBS[index] == 'm' || maxClientBS[index] == 'M')
			{
				_maxBodySize = _maxBodySize << 20;
			}
			else
				throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong unit, must be m, M, k or K");
			std::cerr << "max client body size = " << _maxBodySize << '\n';
		}
		else
			throw ErrorConfigFile("Error in the conf file : max_client_body_size : invalid character after the unit size");
	}
	if (_maxBodySize < 0 || _maxBodySize > 2000000)// limite est de 2MB
		throw ErrorConfigFile("Error in the conf file : max_body_client_size : size has to be between 0 and 1MB");
}

void	VirtualServer::parseErrorPages(std::istringstream& iss)
{
	std::string	code;
	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : error_page : missing informations");
	// parseErrorCode();
}

int&	VirtualServer::getPort()
{
	return (_port);
}

void	VirtualServer::setPort(int port)
{
	_port = port;
}

int&	VirtualServer::getFd()
{
	return (_socketfd);
}

void	VirtualServer::setfd(int fd)
{
	_socketfd = fd;
}

void	VirtualServer::connectVirtualServers()
{
// 	for (size_t i = 0; i < _virtualServers.size(); i++)
// 	{

		// struct sockaddr_in sa;
		int socket_fd;
		int status;

		// Prepare the address and port for the server socket
		// memset(&sa, 0, sizeof sa);
		// sa.sin_family = AF_INET; // IPv4
		// sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
		// sa.sin_port = htons(_virtualServers[i].getPort());

		memset(&_address, 0, sizeof _address);
		_address.sin_family = AF_INET; // IPv4
		_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
		_address.sin_port = htons(_port);

		// Create the socket
		socket_fd = socket(_address.sin_family, SOCK_STREAM, 0);
		if (socket_fd == -1)
			callException(-1);
		fcntl(socket_fd, F_SETFL, O_NONBLOCK);
		printf("[Server] Created server socket fd: %d\n", socket_fd);

		// Bind socket to address and port
		status = bind(socket_fd, (struct sockaddr *)&_address, sizeof _address);;
		if (status != 0)
		{
			callException(-1);
		}
		printf("[Server] Bound socket to localhost port %d\n", _port);
		status = listen(socket_fd, 10); // A MODIF
		if (status != 0)
			callException(-1);
		_socketfd = socket_fd;
		// _virtualServers[i].setfd(socket_fd);
	// }
}