#include "../includes/Response.hpp"

Response::Response()
{
	fillStatusMsg();
}

Response::~Response()
{
}

void			Response::fillStatusMsg()
{
	_statusMsg[STATUS_NONE] = "OK"; // A MODIF
	_statusMsg[STATUS_OK] = "OK"; // A MODIF
	_statusMsg[STATUS_MOVED_PERMANENTLY] = "Moved Permanently";
	_statusMsg[STATUS_FORBIDDEN] = "Forbidden";
	_statusMsg[STATUS_BAD_REQUEST] = "Bad Request";
	_statusMsg[STATUS_NOT_FOUND] = "Not Found";
	_statusMsg[STATUS_REQUEST_TIMEOUT] = "Request Time-out";
	_statusMsg[STATUS_PAYLOAD_TOO_LARGE] = "Request Entity Too Large";
	_statusMsg[STATUS_URI_TOO_LONG] = "Request-URI Too Long";
	_statusMsg[STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
	_statusMsg[STATUS_NOT_IMPLEMENTED] = "Not Implemented";
	_statusMsg[STATUS_HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version not supported";
}

void	Response::generateResponse(ParseRequestResult &request)
{
	// MODIFIER LES ELSE IF
	if (request.outcome == REQUEST_FAILURE) //parsing failure
	{
		buildErrorPage(request, request.statusCode);
	}
	else if (0) // CGI
	{
		(void) request;
	}
	else if (0) // location method not allowed
	{
		(void) request;
	}
	else if (0) // redirection return 
	{
		(void) request;
	}
	else
	{
		if (request.method == GET)
			buildGet(request);
		if (request.method == POST)
			return ; // buildPost(request)
		if (request.method == DELETE)
			return ; // buildDelete(request)
	}

	_statusCode = request.statusCode;
	if (_statusCode == STATUS_NONE)
		_statusCode = STATUS_OK; // A enlever
	buildStatusLine();
	// build headers (+body)
}

void			Response::buildStatusLine()
{
	std::stringstream ss;
	ss << _statusCode;
	_statusLine = std::string(PROTOCOL_VERSION) + " " + ss.str() + " " + _statusMsg[_statusCode] + "\r\n";
	// std::cout << GREEN << "statusLine = " << _statusLine << RESET << std::endl;
}

void			Response::buildErrorPage(ParseRequestResult &request, StatusCode statusCode)
{
	// Attention, le request.statusCode n'est plus forcement valide => utilise celui envoye dans les arguments
	_statusCode = statusCode;
		// error_page in location 
		// or
		// build error page from scratch
}

void	Response::buildGet(ParseRequestResult &request)
{
	_configLocation = request.location->getConfigLocation();
	if (_configLocation.find("rootDir") != _configLocation.end())
		_rootDir = _configLocation["rootDir"][0];
	std::cout << "root = " << _rootDir << std::endl;
	if (_rootDir.back() != '/')
		_rootDir.pop_back();
	_finalURI = _rootDir + request.uri;

	if (isUriValid(_finalURI) == false)
	{
		return (buildErrorPage(request, STATUS_FORBIDDEN));
	}
	if (isPathADirectory(_finalURI))
	{
		if (_finalURI.back() != '/')
		{
			// request.statusCode = STATUS_MOVED_PERMANENTLY;
			_statusCode = STATUS_MOVED_PERMANENTLY;
			_headers["location"] = "http://" + request.hostName + request.uri + "\r\n"; //A mettre ici ou dans builHeaders ?
			return ;
		}
		else
		{
			if (_configLocation.find("_indexPages") != _configLocation.end())
			{
				std::vector<std::string> indexPages = _configLocation["_indexPages"];
				Location *newLocation = NULL;
				if (indexPages.empty() == false)
				{
					for (std::vector<std::string>::iterator it = indexPages.begin(); it != indexPages.end(); it++)
					{
						std::string index = (*it)[0] == '/' ? (*it).substr(1, std::string::npos) : (*it);
						std::string path;
						path = _finalURI + index;
						if (isPathADRegularFile(path))
						{
							_finalURI = path;
							break ;
						}
						if (request.location->getEqualModifier() == true && newLocation == NULL)
						{
							newLocation = associateLocationResponse(request, index);
						}
					}
				}
				if (newLocation)
				{
					request.location = newLocation;
					return (generateResponse(request));
				}
			}
			if (_configLocation.find("autoindex") != _configLocation.end()
					&& _configLocation["autoindex"][0] == "true")
			{
					buildAutoindexPage();
			}
			buildErrorPage(request, STATUS_FORBIDDEN);
			return ;
		}
	}
	if (isPathADRegularFile(_finalURI))
		buildPage(request);
	else
		buildErrorPage(request, STATUS_NOT_FOUND);	
}

void			Response::buildPage(ParseRequestResult &request)
{
	std::ifstream fileRequested(_finalURI.c_str());
	if (fileRequested.good() == false)
	{
		return(buildErrorPage(request, STATUS_NOT_FOUND));
	}
	std::stringstream buffer;
	buffer << fileRequested.rdbuf();
	_body = buffer.str();


}

void			Response::buildAutoindexPage()
{

}

ResponseOutcome	Response::sendResponseToClient(int fd)
{
	std::string	line;

	if (pushStrToClient(fd, _statusLine) == -1)
		return RESPONSE_FAILURE;

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	{
		line = it->first + ": " + it->second + "\r\n";
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
	}
	line = "\r\n";
	if (pushStrToClient(fd, line) == -1)
		return RESPONSE_FAILURE;
	
	// METHOD HEAD => stop and return success

	std::map<std::string, std::string>::iterator body = _headers.find("content-length");
	if (body != _headers.end() && strtol(body->second.c_str(), NULL, 10) > 0)
	{
		line = _body.substr(0, strtol(body->second.c_str(), NULL, 10) - 1); //convertir
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
		return RESPONSE_SUCCESS;
	}
	else 
		return (RESPONSE_SUCCESS);
	return RESPONSE_PENDING;
}

int	Response::pushStrToClient(int fd, std::string &str)
{
	size_t	bytesSent = 0, tmpSent = 0;

	while (bytesSent < str.size())
	{
		tmpSent = send(fd, str.c_str() + bytesSent, str.size() - bytesSent, 0);
		if (tmpSent <= 0)
			return (-1);
		bytesSent += tmpSent;
	}
	return (0);
}

Location	*Response::associateLocationResponse(ParseRequestResult &request, std::string index)
{
	size_t len(0);
	std::string	newURI = request.uri + index;
	Location *newLoc = NULL;

	// exact match
	for (std::map<std::string, Location>::iterator it = request.vs->getLocations().begin(); it != request.vs->getLocations().end(); it++)
	{
		if (it->second.getEqualModifier() == true)
		{
			if (it->first == newURI)
			{
				request.uri = newURI;
				return (&(it->second));
			}	
		}
	}

	// longuest prefix
	for (std::map<std::string, Location>::iterator it = request.vs->getLocations().begin(); it != request.vs->getLocations().end(); it++)
	{
		if (it->second.getEqualModifier() == false)
		{
			if (newURI.substr(0, it->first.size()) == it->first)
			{
				if (it->first.size() > len)
				{
					request.uri = newURI;
					newLoc = &(it->second);
					len = it->first.size();
				}
			}	
		}
	}

	return (newLoc);
}