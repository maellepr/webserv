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
	_statusMsg[STATUS_BAD_REQUEST] = "Not Found";
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
		// predefined error page 
		// or
		// build error page from scratch
	}
	else if (0) // CGI
	{
		(void) request;
	}
	else if (0) // location method not allowed
	{
		(void) request; // idem 1
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

void	Response::buildGet(ParseRequestResult &request)
{
	std::map<std::string, std::vector<std::string> >::iterator it = request.location->getConfigLocation().find("rootDir");
	if (it != request.location->getConfigLocation().end())
		_finalURI = it->second[0];
	std::cout << "root = " << _finalURI << std::endl;
	if (_finalURI.back() != '/')
		_finalURI.pop_back();
	_finalURI += request.uri;

	if (isPathADirectory(_finalURI))
	{
		if (_finalURI.back() != '/')
		{
			_statusCode = STATUS_MOVED_PERMANENTLY;
			_headers["location"] = "http://" + request.hostName + request.uri + "\r\n"; //A mettre ici ou dans builHeaders ?
			return ;
		}
		else
		{
			// index ou autoindex ?
				// servir ressource OU errror
		}
	}
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