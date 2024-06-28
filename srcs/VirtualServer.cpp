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

void	VirtualServer::init(std::istream & file)
{
	// _port = 8080;
	// _serverName = "www.minigoats.com";
	std::string	line, keyword;
	while (std::getline(file, line))
	{
		std::cerr << "line vs = " << line << "\n";
		std::istringstream iss(line);

		iss >> keyword;
		// std::cerr << "keyword vs = " << keyword << "\n";

		// if (!(iss >> keyword))
		// 	continue ;
		if (keyword == "listen")
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
			// parseClient
		}
		else if (keyword == "error_page")
		{

		}
		else if (keyword == "index")
		{

		}
		else if (keyword == "location")
		{

		}
		else
			throw ErrorConfigFile("Error in the config file : wrong keyword");
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
		sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		return ;
	}
	if (_ip == "*")
	{
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
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
	sa.sin_addr.s_addr = htonl(res);
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