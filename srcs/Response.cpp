#include "../includes/Response.hpp"

Response::Response()
{
}

Response::~Response()
{
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
			return ; // buildGet(request)
		if (request.method == POST)
			return ; // buildPost(request)
		if (request.method == DELETE)
			return ; // buildDelete(request)
	}

	// build response line
	// build headers (+body)
}



ResponseOutcome	Response::sendResponseToClient(int fd)
{
	std::string	line;

	if (pushStrToClient(fd, _responseLine) == -1)
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