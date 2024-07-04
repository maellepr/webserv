#include "../includes/Client.hpp"

// CONSTRUCTORS / DESTRUCTORS ------------------------------------------------------ //

Client::Client() : _parsingStep(IN_REQUESTLINE) {}

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
    _clientfd = fd;
}

int Client::getFd()
{
    return _clientfd;
}

void	Client::setConnectedServers(int serverfd, std::map<int, std::vector<VirtualServer*> >	&socketBoundVs)
{
	std::map<int, std::vector<VirtualServer*> >::iterator it = socketBoundVs.find(serverfd);
	if (it != socketBoundVs.end())
		std::copy(it->second.begin(), it->second.end(), _vsCandidates.begin());
}

// VirtualServer &Client::getConnectedServer()
// {
//     return ;
// }

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
	(void) _serversfd; //A ENLEVER
	(void) _parsingStep; //A ENLEVER
	dprintf(2, "read data from socket [%d]\n", _clientfd);

	char	buffer[BUFSIZ];// A MODIF
	size_t	bytesRead;
	ParseRequestResult	reqRes;

	memset(&buffer, '\0', sizeof(buffer));
	bytesRead = recv(_clientfd, buffer, BUFSIZ, 0);
	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			printf("[%d] Client socket closed connection.\n", _clientfd);
		else
			fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
		close(_clientfd);
		return (-1);
	}
	else 
	{
		size_t addBytes = bytesRead;
		while (addBytes)
		{
			_buffer += std::string(buffer, addBytes);
			memset(&buffer, '\0', sizeof(buffer));
			addBytes = recv(_clientfd, buffer, BUFSIZ, 0);
			if (addBytes < 0)
			{
				fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
				close(_clientfd);
				return (-1);
			}
		}
		printf("[%d] Got message: %s", _clientfd, buffer);// buffer A PARSER
		if (_request == NULL)
			_request = new Request(); // A PROTEGER ?

	}
	_buffer += buffer;
	reqRes = _request->parseBuffer(_buffer); // FATAL ERRORS ?
	if (reqRes.outcome == REQUEST_PENDING) // TIMEOUT TO DO
			return (0);

	// ECRIRE RESULTAT PARSING

	_response = new Response; //A PROTEGER?
	_response->generateResponse(reqRes);

	delete _request;
	_request = NULL;
	return (0);
}

ResponseOutcome Client::writeResponse()
{
    ResponseOutcome status;

    // status = write(_clientfd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
    // if (status == -1)
    //     return (-1);

	status = _response->sendResponseToClient(_clientfd);
	if (status != RESPONSE_PENDING)
	{
		delete _response;
		_response = NULL;
	}

    return (status);
}