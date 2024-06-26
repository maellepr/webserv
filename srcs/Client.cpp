#include "../includes/Client.hpp"

Client::Client()
{

}

Client::~Client()
{

}

void	Client::setFd(int fd)
{
    _socketfd = fd;
}

int Client::getFd()
{
    return _socketfd;
}

int Client::readRequest()
{
	dprintf(2, "read data from socket [%d]\n", _socketfd);

	char	buffer[BUFSIZ];// A MODIF
	size_t	bytesRead;
	int	parseRequestResult;

	memset(&buffer, '\0', sizeof(buffer));
	bytesRead = recv(_socketfd, buffer, BUFSIZ, 0);
	if (bytesRead <= 0) {
		if (bytesRead == 0)
			printf("[%d] Client socket closed connection.\n", _socketfd);
		else
			fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
		close(_socketfd); // A VOIR ??
		return (-1);
	}
	else 
	{
		printf("[%d] Got message: %s", _socketfd, buffer);// buffer A PARSER
		if (_request == NULL)
			_request = new Request; // A PROTEGER ?
	}
	parseRequestResult = _request->parse(buffer, bytesRead);

	delete _request;
	_request = NULL;
	dprintf(2, "read data 4\n");
	return (0);
}

int Client::writeResponse()
{
    int status;

    status = write(_socketfd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
    if (status == -1) {
        return (-1);
        // fprintf(stderr, "[Server] Send error to client fd 4: %s\n", strerror(errno));
    }
    dprintf(2, "send data\n");
    // close(_socketfd);
    return (0);
}
