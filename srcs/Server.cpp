#include "../includes/webserv.hpp"
#include "../includes/Server.hpp"

Server::Server()
{

}

Server::~Server()
{

}

bool	Server::init(const char *filename)
{
	// parse to do
	VirtualServer vs;
	if (!vs.init())
		return false;
	_virtualServers.push_back(vs);

	connectVirtualServers();
	return true;
}

void	Server::connectVirtualServers()
{
for (size_t i = 0; i < 1; i++)
	{
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
		status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);
		if (status != 0)
			callException(-1);
		printf("[Server] Bound socket to localhost port %d\n", _virtualServers[i].getPort());

		status = listen(socket_fd, 10); // A MODIF
			callException(-1);

		_virtualServers[i].setfd(socket_fd);
	}
}

void	Server::loop()
{
	
	for (size_t i = 0; i < 1; i++)
	{


    FD_ZERO(&all_sockets);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(server_socket, &all_sockets); // Add listener socket to set
    fd_max = server_socket; // Highest fd is necessarily our socket
	
	while (1)
	{

	}
	}

}