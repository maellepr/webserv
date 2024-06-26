#include "../includes/webserv.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

Server::Server()
{

}

Server::~Server()
{

}

bool	Server::init(const char *filename)
{
	// parse to do
	(void) filename;
	VirtualServer vs;
	if (!vs.init())
		return false;
	_virtualServers.push_back(vs);

	VirtualServer vs2;
	vs2.setPort(1234);
	_virtualServers.push_back(vs2);

	VirtualServer vs3;
	vs3.setPort(12345);
	_virtualServers.push_back(vs3);

	connectVirtualServers();
	return true;
}

void	Server::connectVirtualServers()
{
	for (size_t i = 0; i < _virtualServers.size(); i++)
	{
		dprintf(2, "VS numero %lu, port %d\n", i, _virtualServers[i].getPort());
		struct sockaddr_in sa;
		int socket_fd;
		int status;

		// Prepare the address and port for the server socket
		memset(&sa, 0, sizeof sa);
		sa.sin_family = AF_INET; // IPv4
		sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
		sa.sin_port = htons(_virtualServers[i].getPort());

		// Create the socket
		socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
		if (socket_fd == -1)
			callException(-1);
		fcntl(socket_fd, F_SETFL, O_NONBLOCK);
		printf("[Server] Created server socket fd: %d\n", socket_fd);

		// Bind socket to address and port
		status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);;
		if (status != 0)
			callException(-1);
		printf("[Server] Bound socket to localhost port %d\n", _virtualServers[i].getPort());
		status = listen(socket_fd, 10); // A MODIF
		if (status != 0)
			callException(-1);
		_virtualServers[i].setfd(socket_fd);
	}
}

void	Server::loop()
{

	int			status;
	int			res = 0;
	struct timeval timer;
	
	FD_ZERO(&_all_sockets);
	FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);
	_fd_max = 0;
	for (size_t i = 0; i < _virtualServers.size(); i++)
	{
		FD_SET(_virtualServers[i].getFd(), &_all_sockets); // Add listener socket to set
		if (_fd_max < _virtualServers[i].getFd())
			_fd_max = _virtualServers[i].getFd();
	}
	dprintf(2, "fd max du debut = %d\n", _fd_max);
	
	while (1)
	{
    	dprintf(2, "while 1\n");
        sleep(2);
        _read_fds = _all_sockets;
        _write_fds = _all_sockets;
        // 2 second timeout for select()
        timer.tv_sec = 10;
        timer.tv_usec = 0;
        
        dprintf(2, "while 2\n");
        status = select(_fd_max + 1, &_read_fds, &_write_fds, NULL, &timer);
        if (status == -1)
            callException(-2);
        else if (status == 0) {
            // No socket fd is ready to read
            printf("[Server] Waiting...\n");
            continue;
        }
        dprintf(2, "while 3\n");
        dprintf(2, "_fd_max = %d\n", _fd_max);
        for (int i = 0; i <= _fd_max; i++) 
        {
            dprintf(2, "while 4\n");
            if (FD_ISSET(i, &_read_fds) != 1) {
                // Fd i is not a socket to monitor
                // stop here and continue the loop
                continue ;
            }
            printf("[%d] Ready for I/O operation\n", i);
            // Socket is ready to read!
			size_t j = 0;
			for (;j < _virtualServers.size(); j++)
			{
				if (i == _virtualServers[j].getFd())
				{
					res = 1;
					break;
				}
			}
			if (res)
			{
				// Socket is our server's listener socket
				Client client;
		
				dprintf(2, "while 5\n");
				client.setFd(_acceptNewConnection(_virtualServers[j].getFd()));
				_clients[client.getFd()] = client;
				res = 0;
			}
            else {
                // Socket is a client socket, let's read it
                Client&	client = _clients[i]; 
				
				dprintf(2, "while 6\n");
                if (client.readRequest() == -1)
				{
					FD_CLR(i, &_all_sockets); // Remove socket from the set
					_clients.erase(i);
					if (i == _fd_max)
						_fd_max = _fd_max - 1; //TEMPORAIRE A MODIF
				}
            }
        }
        for (int i = 0; i <= _fd_max; i++) 
        {
            dprintf(2, "while 7\n");
            if (FD_ISSET(i, &_write_fds) != 1)
                continue ;
            else
			{
                Client&	client = _clients[i];
				// Socket is a client socket, let's read it
                dprintf(2, "socket client pret a write\n");
                if (client.writeResponse() == -1)
				{
					close(i);
					FD_CLR(i, &_all_sockets);
					_clients.erase(i);
					if (i == _fd_max)
						_fd_max = _fd_max - 1; //TEMPORAIRE A MODIF
				}
            }
        }
	}
}

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

