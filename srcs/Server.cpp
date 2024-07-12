#include "../includes/webserv.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

Server::Server()
{

}

Server::~Server()
{

}

void	Server::init(const char *filename)
{
	extension(filename, ".conf");
	isDirectory(filename);
	fillStatusMsg();

	std::ifstream file(filename);
	if (!file.is_open())
		throw ErrorConfigFile("Error : file couldn't open");

	std::string	line;
	std::vector <VirtualServer> virtualServersTemp;
	int	i = 0;
	while (std::getline(file, line))
	{
		if (line == "server {")
		{
			VirtualServer vs;	
			vs.init(file);
			vs.setIndex(i);
			virtualServersTemp.push_back(vs);
			i++;
		}
		else if (line.empty())
			continue ;
		else
			throw ErrorConfigFile("Error in the config file : wrong content");
	}
	if (virtualServersTemp.size() == 0)
		throw ErrorConfigFile("Error : file empty");
	_nbServers = i;
	_eraseVSIfDuplicate(virtualServersTemp);
	if (_virtualServers.size() == 0)
		throw ErrorConfigFile("Error in the config file : invalid servers");
	_prepareVSToConnect();
	
	for (size_t i = 0; i < _virtualServers.size(); i++)
	{
		if (_virtualServers[i].getIsBind() == 0)
		{
			std::vector<VirtualServer*>	bindedVS;

			_virtualServers[i].connectVirtualServers();
			bindedVS.push_back(&_virtualServers[i]);
			_addBindedVS(i, bindedVS);
			_socketBoundVs[_virtualServers[i].getSocketFd()] = bindedVS;
		}
	}
	// std::cerr << "1. Check _socketBoundVS :\n";
	// for (std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.begin(); it != _socketBoundVs.end(); it++) 
	// {
    //     std::cout << "Socket: " << it->first << std::endl;
    //     for (std::vector<VirtualServer*>::iterator vs = it->second.begin(); vs != it->second.end(); ++vs) {
    //         std::cout << " virtual server name : " << (*vs)->getServerName() << " ip : " << (*vs)->getIP() << " port : " << (*vs)->getPort() << " vs default = " << (*vs)->getDefaultVS() << std::endl;
    //     }
    // }
	_checkDuplicateDefaultServer();
	// _checkNecessaryServerName();
	std::cerr << "2. Check _socketBoundVS :\n";
	for (std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.begin(); it != _socketBoundVs.end(); it++) 
	{
        std::cout << "Socket: " << it->first << std::endl;
        for (std::vector<VirtualServer*>::iterator vs = it->second.begin(); vs != it->second.end(); ++vs) {
            std::cout << " virtual server name : " << (*vs)->getServerName() << " ip : " << (*vs)->getIP() << " port : " << (*vs)->getPort() << " vs default = " << (*vs)->getDefaultVS() << std::endl;
        }
    }
}

void	Server::_eraseVSIfDuplicate(std::vector<VirtualServer> &virtualServersTemp)
{
	// for (size_t i = 0; i < virtualServersTemp.size(); i++)
	// {	
		// std::cerr << "Apres init : ip = " << _virtualServers[i].getIP() << " port = " << _virtualServers[i].getPort() << " server name = " << _virtualServers[i].getServerName() << " toErase = " << _virtualServers[i].getToErase() << "\n";
	// }
	// std::cerr << "\n\n";
	for (size_t i = 0; i < virtualServersTemp.size(); i++)
	{
		for (size_t j = 0; j < virtualServersTemp.size(); j++)
		{
			if (i != j)
			{
				if (virtualServersTemp[i].getIP() == virtualServersTemp[j].getIP() 
				&& virtualServersTemp[i].getPort() == virtualServersTemp[j].getPort() 
				&& virtualServersTemp[i].getServerName() == virtualServersTemp[j].getServerName())
				{
					virtualServersTemp[j].setToErase(true);
					virtualServersTemp[i].setToErase(true);
				}
			}
		}
	}
	// std::cerr << "_virtualServer AVANT erase :\n";
	// for (size_t i = 0; i < virtualServersTemp.size(); i++)
	// {	
	// 		std::cerr << "XXX ip = " << virtualServersTemp[i].getIP() << " port = " << virtualServersTemp[i].getPort() << " server name = " << virtualServersTemp[i].getServerName() << " toErase = " << virtualServersTemp[i].getToErase() << "\n";
	// }
	// std::cerr << "\n";
	for (size_t i = 0; i < virtualServersTemp.size(); i++)
	{
		if (virtualServersTemp[i].getToErase() == false)
			_virtualServers.push_back(virtualServersTemp[i]);
	}
	// std::cerr << "_virtualServer APRES erase :\n";
	// for (size_t i = 0; i < _virtualServers.size(); i++)
	// {	
	// 		std::cerr << "ip = " << _virtualServers[i].getIP() << " port = " << _virtualServers[i].getPort() << " server name = " << _virtualServers[i].getServerName() << " toErase = " << _virtualServers[i].getToErase() << " index " << _virtualServers[i].getIndex() << " default_server = " << _virtualServers[i].getDefaultVS() << "\n";
	// }
	// std::cerr << "\n";
}

void	Server::_prepareVSToConnect()
{
	// for (size_t i = 0; i < _virtualServers.size(); i++)
		// std::cerr << "VS ip = " << _virtualServers[i].getIP() << " port = " << _virtualServers[i].getPort() << "\n";
	for (size_t i = 0; i < _virtualServers.size(); i++)
	{
		if (_virtualServers[i].getIP() != "0.0.0.0")
			_ipIsSpecificAddress(i);
	}
	for (size_t i = 0; i < _virtualServers.size(); i++)
	{
		if (_virtualServers[i].getIP() == "0.0.0.0")
			_ipIsAnyAddress(i);
	}
	// for (size_t i = 0; i < _virtualServers.size(); i++)
		// std::cerr << "VS ip = " << _virtualServers[i].getIP() << " port = " << _virtualServers[i].getPort() << " server name = " << _virtualServers[i].getServerName() << " isBind = " << _virtualServers[i].getIsBind() << " default_server = " << _virtualServers[i].getDefaultVS() << "\n";
}

void	Server::_ipIsAnyAddress(size_t i)
{
	for (size_t j = 0; j < _virtualServers.size(); j++)
	{
		if (j != i)
		{
			if (_virtualServers[i].getIP() == _virtualServers[j].getIP() && _virtualServers[i].getPort() == _virtualServers[j].getPort())
			{
				if (_virtualServers[i].getServerName() == _virtualServers[j].getServerName())
					throw ErrorConfigFile("Error : two blocks server have same port, ip address and server_name");
				if (_virtualServers[i].getIsBind() == 0)
					_virtualServers[j].setIsBind(1);
			}
			if (_virtualServers[i].getPort() == _virtualServers[j].getPort())
				_virtualServers[j].setIsBind(1);
		}
	}
}

void	Server::_ipIsSpecificAddress(size_t i)
{
	for (size_t j = 0; j < _virtualServers.size(); j++)
	{
		if (j != i)
		{
			if (_virtualServers[i].getIP() == _virtualServers[j].getIP() && _virtualServers[i].getPort() == _virtualServers[j].getPort())
			{				
				// std::cerr << "i = " << i << " j =" <<  j << "\n";
				// std::cerr << "ip = " << _virtualServers[i].getIP() << " " <<  _virtualServers[j].getIP() << "\n";
				// std::cerr << "port = " << _virtualServers[i].getPort() << " " <<  _virtualServers[j].getPort() << "\n";
				// std::cerr << "server_name = " << _virtualServers[i].getServerName() << " " <<  _virtualServers[j].getServerName() << "\n";
				if (_virtualServers[i].getServerName() == _virtualServers[j].getServerName())
					throw ErrorConfigFile("Error : two blocks server have same port / ip address / server_name");
				if (_virtualServers[i].getIsBind() == 0)
					_virtualServers[j].setIsBind(1);
			}
		}
	}
}

void	Server::_checkDuplicateDefaultServer()
{
	// int	nbDefaultVS = 0;

	// _socketBoundVs

	for (std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.begin(); it != _socketBoundVs.end(); it++) 
	{	
		// std::cerr << "fd = " << it->first << "\n";
        // std::cout << "Socket: " << it->first << std::endl;
        int nbDefaultVS = 0;
		for (std::vector<VirtualServer*>::iterator vs = it->second.begin(); vs != it->second.end(); vs++)
		{	
			// std::cout << " virtual server name : " << (*vs)->getServerName() << " ip : " << (*vs)->getIP() << " port : " << (*vs)->getPort() << " defaultVS = " << (*vs)->getDefaultVS() << std::endl;
			
			if ((*vs)->getDefaultVS() == true)
			{
				nbDefaultVS++;
			}
		}
		// std::cerr << "nbDefaultVS = " << nbDefaultVS << "\n";
		if (nbDefaultVS > 1)
			throw ErrorConfigFile("Error in the conf file : several servers set up as server by default (in the same bind)");
		
		if (nbDefaultVS == 0)
		{
			_defineVSByDefault(it);
		}

	}

	// for (size_t i = 0; i < _virtualServers.size(); i++)
	// {
		// if (_virtualServers[i].getDefaultVS() == true)
			// nbDefaultVS++;
	// }
	// if (nbDefaultVS > 1)
		// throw ErrorConfigFile("Error in the conf file : there is several servers set up as server by default");
}

void	Server::_defineVSByDefault(std::map<int, std::vector<VirtualServer*> >::iterator it)
{
	int	minServer = _nbServers;
	for (std::vector<VirtualServer*>::iterator vs = it->second.begin(); vs != it->second.end(); vs++)
	{
		if ((*vs)->getIndex() < minServer)
		{
			minServer = (*vs)->getIndex();
		}
	}
	for (std::vector<VirtualServer*>::iterator vs = it->second.begin(); vs != it->second.end(); vs++)
	{
		if ((*vs)->getIndex() == minServer)
			(*vs)->setDefaultVS(true);
	}
}

// void	Server::connectVirtualServers()
// {
// 	for (size_t i = 0; i < _virtualServers.size(); i++)
// 	{
// 		dprintf(2, "VS numero %lu, port %d\n", i, _virtualServers[i].getPort());
// 		// struct sockaddr_in sa;
// 		int socket_fd;
// 		int status;

// 		// Prepare the address and port for the server socket
// 		// memset(&sa, 0, sizeof sa);
// 		// sa.sin_family = AF_INET; // IPv4
// 		// sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
// 		// sa.sin_port = htons(_virtualServers[i].getPort());

// 		// Create the socket
// 		socket_fd = socket(_virtualServers[i].sa.sin_family, SOCK_STREAM, 0);
// 		if (socket_fd == -1)
// 			callException(-1);
// 		fcntl(socket_fd, F_SETFL, O_NONBLOCK);
// 		printf("[Server] Created server socket fd: %d\n", socket_fd);

// 		// Bind socket to address and port
// 		status = bind(socket_fd, (struct sockaddr *)&_virtualServers[i].sa, sizeof _virtualServers[i].sa);;
// 		if (status != 0)
// 			callException(-1);
// 		printf("[Server] Bound socket to localhost port %d\n", _virtualServers[i].getPort());
// 		status = listen(socket_fd, 10); // A MODIF
// 		if (status != 0)
// 			callException(-1);
// 		_virtualServers[i].setfd(socket_fd);
// 	}
// }

int	Server::_acceptNewConnection(int server_socket)
{
    int client_fd;

    client_fd = accept(server_socket, NULL, NULL);
    if (client_fd == -1)
		callException(-3);
    FD_SET(client_fd, &_all_sockets); // Add the new client socket to the set
    if (client_fd > _fd_max) {
        _fd_max = client_fd; // Update the highest socket
    }
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    printf("[Server] Accepted new connection on client socket %d.\n", client_fd);
	return (client_fd);
}

void	Server::_addBindedVS(size_t i, std::vector<VirtualServer*> &bindedVS)
{
	for (size_t j = 0; j < _virtualServers.size(); j++)
	{
		if (i != j && _virtualServers[j].getIsBind() == 1)
		{
			if (_virtualServers[i].getIP() == "0.0.0.0" 
				&& _virtualServers[i].getPort() == _virtualServers[j].getPort())
				bindedVS.push_back(&_virtualServers[j]);
			else if (_virtualServers[i].getPort() == _virtualServers[j].getPort())
				bindedVS.push_back(&_virtualServers[j]);
		}
	}
}

void	Server::loop()
{

	int			status;
	// int			res = 0;
	struct timeval timer;
	
	FD_ZERO(&_all_sockets);
	FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);
	_fd_max = -1;
	for (std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.begin(); it != _socketBoundVs.end(); it++)
	{
		FD_SET(it->first, &_all_sockets);
		if (_fd_max < it->first)
			_fd_max = it->first;
	}
	// for (size_t i = 0; i < _virtualServers.size(); i++)
	// {
	// 	FD_SET(_virtualServers[i].getFd(), &_all_sockets); // Add listener socket to set
	// 	if (_fd_max < _virtualServers[i].getFd())
	// 		_fd_max = _virtualServers[i].getFd();
	// }
	dprintf(2, "fd max du debut = %d\n", _fd_max);
	
	while (1)
	{
    	dprintf(2, "WHILE 1 - before sleep\n");
        sleep(2); // A ENLEVER
        _read_fds = _all_sockets;
        _write_fds = _all_sockets;
        timer.tv_sec = 2; // 2 second timeout for select()
        timer.tv_usec = 0;
        
        dprintf(2, "WHILE 2 - avant select\n");
        status = select(_fd_max + 1, &_read_fds, &_write_fds, NULL, &timer);
        if (status == -1)
            callException(-2); // A modifier car le server ne doit pas s'arreter ?
        else if (status == 0)
		{
            printf("[Server] Waiting...\n");
            continue;
        }
        dprintf(2, "WHILE 3 - apres select\n");
        dprintf(2, "_fd_max = %d\n", _fd_max);
        for (int i = 0; i <= _fd_max && status > 0; i++) // 1 seule boucle ou 2 ?
        {
            if (FD_ISSET(i, &_read_fds) == 1)
			{
				dprintf(2, "WHILE 4 - debut boucle read set\n");
				status--;
				// size_t j = 0;
				// for (;j < _virtualServers.size(); j++)
				// {
				// 	if (i == _virtualServers[j].getFd())
				// 	{
				// 		res = 1;
				// 		break;
				// 	}
				// }
				// if (res)
				std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.find(i);
				if (it != _socketBoundVs.end())
				{
					dprintf(2, "WHILE 5 - read socket is a server\n");
					// CHECK NB DE CLIENTS CONNECTES TO DO
					Client client;

					client.setFd(_acceptNewConnection(i));
					client.setConnectedServers(i, _socketBoundVs);
					dprintf(2, "WHILE 5 - 3\n");
					_clients[client.getFd()] = client;
					dprintf(2, "WHILE 5 - 4\n");
					// res = 0;
				}
				else
				{
					dprintf(2, "WHILE 6 - read socket is a client\n");
					Client&	client = _clients[i]; 
					
					if (client.readRequest() == -1)
					{
						FD_CLR(i, &_all_sockets); // Remove socket from the set
						_clients.erase(i);
						if (i == _fd_max)
							_fd_max = _fd_max - 1; //TEMPORAIRE A MODIF
					}
				}
				dprintf(2, "END of READ\n");
      		}
			else if (FD_ISSET(i, &_write_fds) == 1)
			{
				dprintf(2, "WHILE 7 - a client socket is ready to write\n");
				status--;
                Client&	client = _clients[i];
				if (client.writeResponse() == RESPONSE_FAILURE) // A VERIF
				{
					close(i);
					FD_CLR(i, &_all_sockets);
					_clients.erase(i);
					if (i == _fd_max)
						_fd_max = _fd_max - 1; //TEMPORAIRE A MODIF
				}
				dprintf(2, "END of WRITE\n");
        	}
    	}
	}
}

// void	Server::_checkNecessaryServerName()
// {
// 	// std::map<int, std::vector<VirtualServer*> >	_socketBoundVs;
// 	for(std::map<int, std::vector<VirtualServer*> >::iterator sB = _socketBoundVs.begin(); sB != _socketBoundVs.end(); sB++)
// 	{
		
// 	}
// }