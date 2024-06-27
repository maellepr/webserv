#include "../includes/VirtualServer.hpp"

VirtualServer::VirtualServer()
{
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
		break;
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
		// std::cerr << "_port = " << _port << "\n";
	}
	else
	{
		std::string	port = line;
		parsePort(port);
		// std::cerr << "_port else = " << _port << "\n";
	}
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
	int	dots = 0;
	for (int i = 0; _ip[i]; i++)
	{	
		if (_ip[i] = '.')
		{
			_ip.replace(i, 1, 1, ' ');
			dots++;
		}
	}
	if (dots != 3)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)");
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