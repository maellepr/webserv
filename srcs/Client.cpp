#include "../includes/Client.hpp"

// CONSTRUCTORS / DESTRUCTORS ------------------------------------------------------ //

Client::Client() : _clientfd(-1), _request(NULL), _response(NULL), _buffer(""), _clientStatus(NONE), _keepAlive(true) {}

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

void	Client::setServerFd(int fd)
{
    _serverfd = fd;
}

int Client::getServerFd()
{
    return _serverfd;
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

void	Client::setFdInfos(int fdMax, fd_set write_fds, fd_set read_fds)
{
	_fd_max = fdMax;
	_write_fds = write_fds;
	_read_fds = read_fds;
}

void	Client::setClient(std::map<int, Client> &c)
{
	_c = &c;
}

void	Client::setSocketBoundVs(std::map<int, std::vector<VirtualServer*> > &vs)
{
	_socketBoundVs = vs;
}

// FUNCTIONS --------------------------------------------------------------------- //

int Client::readRequest(int isInReadSet)
{
	try {
		ParseRequestResult	parsedRequest;
		if (isInReadSet)
		{
			char	buffer[BUFSIZ];
			size_t	bytesRead;

			memset(&buffer, '\0', sizeof(buffer));
			bytesRead = recv(_clientfd, buffer, BUFSIZ, 0);
			if (bytesRead <= 0)
			{
				// if (bytesRead == 0)
				// {
				// 	// if (CLIENT)
				// 		std::cerr << "[" << DARKYELLOW << _clientfd << "]" << "Client socket closed connection.\n" << RESET << std::endl;
				// }
				// else
				// {
				// 	if (CLIENT)
				// 		std::cerr << "[Server] Recv error: " << strerror(errno) << std::endl;
				// }
				// close(_clientfd);
				return (-1);
			}
			if (DEBUG)
			{
				std::cout << ORANGE << "REQUEST from client socket : " << _clientfd
						// << "\n===============\n"
						// << buffer
						// << "===============" << RESET << std::endl;
						<< "\n===============\n";
						std::string buf(buffer, bytesRead);
						for (std::string::iterator it = buf.begin(); it != buf.end(); it++)
						{
							if ((*it) == '\r')
								std::cout << ORANGE << "CR";
							else if ((*it) == '\n')
								std::cout << ORANGE << "LF" << std::endl;
							else
								std::cout << ORANGE << (*it);
						}
						std::cout << ORANGE << "===============" << RESET << std::endl;
			}
			if (_request == NULL)
			{
				_request = new Request(_clientfd, _vsCandidates);
				_requestStartTime = std::time(NULL);
				_clientStatus = REQUEST_ONGOING;
			}
			_buffer.append(buffer, bytesRead);
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
		else if (parsedRequest.outcome == REQUEST_FAILURE)
		{
			_buffer.erase();
			_clientStatus = TO_CLOSE;
		}

		// if (CLIENT)
			// std::cerr << LIGHTGREEN << "REQUEST OUTCOME = " << parsedRequest.outcome << RESET << std::endl;
		
		if (_clientStatus != TO_CLOSE)
			_clientStatus = NONE;
		_keepAlive = parsedRequest.keepAlive;

		_response = new Response; // new A PROTEGER?
		_response->setFdInfos(_fd_max, _write_fds, _read_fds);
		_response->setSocketBoundVs(_socketBoundVs);
		_response->setClient(_c);
		_response->generateResponse(parsedRequest);

		delete _request;
		_request = NULL;
		return (0);
	}
	catch (std::exception &e)
	{
		return (-1);
	}
}

ResponseOutcome Client::writeResponse()
{
	try {
		ResponseOutcome status(NOTHING_SENT);

		if (_response)
		{
			status = _response->sendResponseToClient(_clientfd);
			if (status != RESPONSE_PENDING)
			{
				if (status == RESPONSE_SUCCESS_KEEPALIVE && _keepAlive == false)
				{
					status = RESPONSE_SUCCESS_CLOSE;
				}
				if (_response->getErrorCloseSocket() == true)
					status = RESPONSE_SUCCESS_CLOSE;
				delete _response;
				_response = NULL;
			}
		}
		return (status);
		}
	catch (std::exception &e)
	{
		return (RESPONSE_FAILURE);
	}
}