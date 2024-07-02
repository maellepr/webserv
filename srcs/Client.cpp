#include "../includes/Client.hpp"

// CONSTRUCTORS / DESTRUCTORS ------------------------------------------------------ //

Client::Client()
{

}

Client::~Client()
{
	if (_request)
		delete _request;
	if (_response)
		delete _response;
}

// GETTERS / SETTERS ------------------------------------------------------------ //

void	Client::setFd(int fd)
{
    _socketfd = fd;
}

int Client::getFd()
{
    return _socketfd;
}

void	Client::setConnectedServer(VirtualServer &vs)
{
    _vs = vs;
}

VirtualServer &Client::getConnectedServer()
{
    return ;
}

// void	Client::setMaxBodySize(size_t maxBodySize)
// {
// 	_maxBodySize = maxBodySize;
// }

// size_t	Client::getMaxBodySize()
// {
// 	return (_maxBodySize);
// }

// FUNCTIONS --------------------------------------------------------------------- //

int Client::readRequest()
{
	dprintf(2, "read data from socket [%d]\n", _socketfd);

	char	buffer[BUFSIZ];// A MODIF
	size_t	bytesRead;
	ParseRequestResult	reqRes;

	memset(&buffer, '\0', sizeof(buffer));
	bytesRead = recv(_socketfd, buffer, BUFSIZ, 0);
	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			printf("[%d] Client socket closed connection.\n", _socketfd);
		else
			fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
		close(_socketfd);
		return (-1);
	}
	else 
	{
		size_t addBytes = bytesRead;
		while (addBytes)
		{
			_buffer += std::string(buffer, addBytes);
			memset(&buffer, '\0', sizeof(buffer));
			addBytes = recv(_socketfd, buffer, BUFSIZ, 0);
			if (addBytes < 0)
			{
				fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
				close(_socketfd);
				return (-1);
			}
		}
		printf("[%d] Got message: %s", _socketfd, buffer);// buffer A PARSER
		if (_request == NULL)
			_request = new Request(_vs); // A PROTEGER ?

	}
	reqRes = _request->parseBuffer(_buffer); // FATAL ERRORS ?
	if (reqRes.outcome == REQUEST_PENDING) // TIMEOUT TO DO
			return (0);

	// ECRIRE RESULTAT PARSING

	_response = new Response; //A PROTEGER?
	_response->generateResponse(reqRes);

	delete _request;
	_request = NULL;
	dprintf(2, "read data 4\n");
	return (0);
}

ResponseOutcome Client::writeResponse()
{
    ResponseOutcome status;

    // status = write(_socketfd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
    // if (status == -1)
    //     return (-1);

	status = _response->sendResponseToClient(_socketfd);
	if (status != RESPONSE_PENDING)
	{
		delete _response;
		_response = NULL;
	}

    return (status);
}