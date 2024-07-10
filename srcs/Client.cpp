#include "../includes/Client.hpp"

// CONSTRUCTORS / DESTRUCTORS ------------------------------------------------------ //

Client::Client() {}

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
	char	buffer[BUFSIZ];// A MODIF
	size_t	bytesRead;
	ParseRequestResult	parsedRequest;

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
				<< "===============\n"
				<< buffer
				<< "\n===============" << RESET << std::endl;
	}
	if (_request == NULL)
	{
		_request = new Request(_clientfd, _vsCandidates); // new A PROTEGER ?
		_requestStartTime = std::time(NULL);
	}
	_buffer += buffer;
	parsedRequest = _request->parseBuffer(_buffer);
	if (parsedRequest.outcome == REQUEST_PENDING)
	{
		if (std::difftime(std::time(NULL), _requestStartTime) > TIMEOUT)
		{
			parsedRequest.outcome = REQUEST_FAILURE;
			parsedRequest.statusCode = STATUS_REQUEST_TIMEOUT;
		}
		else
			return (0);
	}

	_response = new Response; // new A PROTEGER?
	_response->generateResponse(parsedRequest);

	delete _request;
	_request = NULL;
	return (0);
}

ResponseOutcome Client::writeResponse()
{
    ResponseOutcome status = RESPONSE_SUCCESS;
	static int	i;

	if (i == 0)
	{
		++i;
		int stat = write(_clientfd, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
		if (stat == -1)
			return (RESPONSE_FAILURE);
	}

	// status = _response->sendResponseToClient(_clientfd);
	// if (status != RESPONSE_PENDING)
	// {
	// 	delete _response;
	// 	_response = NULL;
	// }

    return (status);
}