#include "../includes/VirtualServer.hpp"

VirtualServer::VirtualServer()
{
}

VirtualServer::~VirtualServer()
{
}

bool	VirtualServer::init(std::istream & file)
{
	// _port = 8080;
	// _serverName = "www.minigoats.com";
	std::string	line, keyword;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		if (!(iss >> keyword))
			continue ;
		else if (keyword == "listen")
		{
			if (!parseListen(iss))
				return false;
		}
	}
	return true;
}

bool	VirtualServer::parseListen(std::istringstream& iss)
{
	std::string	line, adrsHost;
	int	port;
	if (!(iss >> line))
	{
		std::cerr << "Error in the conf file : missing information after listen\n";
		return false;
	}
	size_t	index = line.find(':');
	if (index == line.size())
	{
		std::cerr << "Error in the conf file : missing information after listen\n";
		return false;
	}
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