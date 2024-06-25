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
}

int&	VirtualServer::getPort()
{
	return (_port);
}

void	VirtualServer::setfd(int fd)
{
	_socketfd = fd;
}