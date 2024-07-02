#include "../includes/VirtualServer.hpp"

VirtualServer::VirtualServer()
{
}

VirtualServer::~VirtualServer()
{
}

bool	VirtualServer::init()
{
	_port = 8080;
	_serverName = "www.minigoats.com";
	return true;
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

size_t	VirtualServer::getMaxBodySize()
{
	return _maxBodySize;
}