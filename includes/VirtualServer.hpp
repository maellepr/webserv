#include "webserv.hpp"

class VirtualServer
{
	public:
		VirtualServer();
		~VirtualServer();

	bool	init(void);

	int		&getPort();
	void	setfd(int fd);

	private:
		int			_port;
		std::string	_serverName;
		int			_socketfd;

		fd_set 		_all_sockets;
		fd_set		_read_fds;
		fd_set		_write_fds;
		int     fd_max;
};