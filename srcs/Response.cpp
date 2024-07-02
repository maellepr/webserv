#include "../includes/Response.hpp"

Response::Response()
{
}

Response::~Response()
{
}

void	Response::generateResponse(ParseRequestResult &reqRes)
{
	(void) reqRes;
	// if (reqRes.outcome == FAILURE)
	// 	generateErrorPage();
	// if CGI
	//	do CGI
	// Error with no page?
	// redirection ?

	// build response: line + headers (+body)
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
	
	// METHOD HEAD => return success

	// SEND BODY if any

	if (1) // A MODIFIER
		return (RESPONSE_SUCCESS);
	else
		return RESPONSE_PENDING;
}

int	Response::pushStrToClient(int fd, std::string &str)
{
	size_t	bytesSent, tmpSent = 0;

	while (bytesSent < _responseLine.size())
	{
		tmpSent = send(fd, _responseLine.c_str() + bytesSent, _responseLine.size() - bytesSent, 0);
		if (tmpSent <= 0)
			return (-1);
		bytesSent += tmpSent;
	}
	return (0);
}