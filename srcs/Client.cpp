#include "../includes/Client.hpp"

// CONSTRUCTORS / DESTRUCTORS ------------------------------------------------------ //

Client::Client() : _clientfd(-1), _request(NULL), _response(NULL), _buffer(""), _clientStatus(NONE) {}

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
	{
		_vsCandidates = it->second;
	}
}

ClientStatus	&Client::getClientStatus()
{
	return _clientStatus;
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

int Client::readRequest(int isInReadSet)
{
	ParseRequestResult	parsedRequest;
	if (isInReadSet)
	{
		// std::cout << "USE RECV\n";
		char	buffer[BUFSIZ];// A MODIF
		size_t	bytesRead;

		memset(&buffer, '\0', sizeof(buffer));
		bytesRead = recv(_clientfd, buffer, BUFSIZ, 0);
		if (bytesRead <= 0)
		{
			if (bytesRead == 0)
				std::cout << "[" << _clientfd << "]" << "Client socket closed connection.\n" << std::endl;
			else
				std::cerr << "[Server] Recv error: " << strerror(errno) << std::endl;
			close(_clientfd);
			return (-1);
		}
		if (DEBUG)
		{
			std::cout << ORANGE << "REQUEST from client socket : " << _clientfd
					<< "\n===============\n"
					<< buffer
					<< "===============" << RESET << std::endl;
		}
		if (_request == NULL)
		{
			_request = new Request(_clientfd, _vsCandidates); // new A PROTEGER ?
			_requestStartTime = std::time(NULL);
			_clientStatus = REQUEST_ONGOING;
		}
		_buffer += buffer;
	}
	parsedRequest = _request->parseBuffer(_buffer);
	if (parsedRequest.outcome == REQUEST_PENDING)
	{
		if (std::difftime(std::time(NULL), _requestStartTime) > TIMEOUT)
		{
			parsedRequest.outcome = REQUEST_FAILURE;
			parsedRequest.statusCode = STATUS_REQUEST_TIMEOUT;
			_clientStatus = NONE;
		}
		else
			return (0);
	}

	std::cout << LIGHTGREEN << "REQUEST OUTCOME = " << parsedRequest.outcome << RESET << std::endl;
	
	_clientStatus = NONE;

	_response = new Response; // new A PROTEGER?
	_response->generateResponse(parsedRequest);

	delete _request;
	_request = NULL;
	return (0);
}

ResponseOutcome Client::writeResponse()
{
    ResponseOutcome status = RESPONSE_SUCCESS;

	if (_response)
		status = _response->sendResponseToClient(_clientfd);
	if (status != RESPONSE_PENDING)
	{
		delete _response;
		_response = NULL;
	}
	
    return (status);

	// static int	i;
	// if (i == 0)
	// {
	// 	++i;
	// 	int stat = write(_clientfd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
	// 	if (stat == -1)
	// 		return (RESPONSE_FAILURE);
	// }
	// return (RESPONSE_SUCCESS);
}