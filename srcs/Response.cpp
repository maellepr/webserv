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
	_statusLine = std::string(PROTOCOL_VERSION) + " " + ss.str() + " " + STATUS_MESSAGES[_statusCode] + "\r\n";
	// std::cout << GREEN << "statusLine = " << _statusLine << RESET << std::endl;
}

void			Response::buildErrorPage(ParseRequestResult &request, StatusCode statusCode)
{
	(void) request;
	// Attention, le request.statusCode n'est plus forcement valide => utilise celui envoye dans les arguments
	_statusCode = statusCode;
	std::map<int, std::string>::iterator loc = request.location->getErrorPages().find(_statusCode);
	std::string	errorPageUri = "." + _rootDir + loc->second;
	if (errorPageUri.empty() || !readContent(errorPageUri, _body))
	{
		std::map<StatusCode, std::string>::iterator it = STATUS_MESSAGES.find(_statusCode);
		std::stringstream ss;
		ss << _statusCode;
		std::string	codeStr = ss.str(); 
		std::string	title;
		if (it != STATUS_MESSAGES.end())
			"Unknown error " + codeStr;
		else
			codeStr + " " + it->second;
		_body = "<!DOCTYPE html>\n"
					"<html lang=\"en\">\n"
					"\n"
					"<head>\n"
					"\t<meta charset=\"UTF-8\">\n"
					"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
					"\t<title>" +
					title +
					"</title>\n"
					"\t<style>\n"
					"\t\tbody {\n"
					"\t\t\tbackground-color: #f0f0f0;\n"
					"\t\t\tfont-family: Arial, sans-serif;\n"
					"\t\t}\n"
					"\n"
					"\t\t.container {\n"
					"\t\t\twidth: 80%;\n"
					"\t\t\tmargin: auto;\n"
					"\t\t\ttext-align: center;\n"
					"\t\t\tpadding-top: 20%;\n"
					"\t\t}\n"
					"\n"
					"\t\th1 {\n"
					"\t\t\tcolor: #333;\n"
					"\t\t}\n"
					"\n"
					"\t\tp {\n"
					"\t\t\tcolor: #666;\n"
					"\t\t}\n"
					"\t</style>\n"
					"</head>\n"
					"\n"
					"<body>\n"
					"\t<div class=\"container\">\n"
					"\t\t<h1>" +
					title +
					"</h1>\n"
					"\t\t<a href=\"/\">Go back to root.</a>\n"
					"\t</div>\n"
					"</body>\n"
					"\n"
					"</html>";
	}
	_headers["content-type"] = "text/html";
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
	if (_rootDir[_rootDir.size() -1] != '/')
		_rootDir = _rootDir.substr(0, _rootDir.size() - 1);
	_finalURI = _rootDir + request.uri;

	if (isUriValid(_finalURI) == false)
	{
		return (buildErrorPage(request, STATUS_FORBIDDEN));
	}
	if (isPathADirectory(_finalURI))
	{
		if (_finalURI[_finalURI.size() -1] != '/')
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

void	Response::buildPage(ParseRequestResult &request)
{
	std::ifstream fileRequested(_finalURI.c_str());
	if (fileRequested.good() == false)
	{
		return(buildErrorPage(request, STATUS_NOT_FOUND));
	}
	std::stringstream buffer;
	buffer << fileRequested.rdbuf();
	_body = buffer.str();

	size_t pos = _finalURI.find_last_of("/");
	if (pos != std::string::npos && (_finalURI.begin() + pos + 1) != _finalURI.end())
	{
		std::string resourceName = _finalURI.substr(pos + 1);
		pos = resourceName.find_last_of(".");
		if (pos != std::string::npos && (resourceName.begin() + pos + 1) != resourceName.end())
		{
			std::string fileExtension = resourceName.substr(pos + 1);
			if (CONTENT_TYPES.find(fileExtension) != CONTENT_TYPES.end())		
			{
				_headers["content-type"] = CONTENT_TYPES[fileExtension];
				return ;
			}
		}
	}

}

void	Response::buildAutoindexPage()
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