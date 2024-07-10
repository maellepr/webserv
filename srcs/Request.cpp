#include "../includes/Request.hpp"

Request::Request(int clientfd, std::vector<VirtualServer*> &vsCandidates) : _clientfd(clientfd), _hostNameDefined(false), _vsCandidates(vsCandidates), _vs(NULL), _location(NULL), _contentLength(0), _parsingStep(IN_REQUESTLINE) {}

Request::~Request() {}

ParseRequestResult	Request::parseBuffer(std::string &buffer)
{
	StatusCode	ret;
	GnlStatus	gnl;

	if (_parsingStep == IN_REQUESTLINE)
	{
		// buffer empty ?
		gnl = getNextLine(buffer);
		if (gnl != FOUND_NL)
		{
			if (gnl == BAD_REQUEST)
				return (parsingFailed(STATUS_BAD_REQUEST));
			if (_line.size() > MAX_URI_SIZE)
				return (parsingFailed(STATUS_URI_TOO_LONG));
			return (parsingPending());
		}
		ret = parseRequestLine(_line);
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		_parsingStep = IN_HEADERS;
		_line.clear();
	}
	if (_parsingStep == IN_HEADERS)
	{
		while (buffer.empty() == false)
		{
			gnl = getNextLine(buffer);
			if (gnl != FOUND_NL)
			{
				if (gnl == BAD_REQUEST)
					return (parsingFailed(STATUS_BAD_REQUEST));
				if (_line.size() > MAX_HEADER_SIZE)
					return (parsingFailed(STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE));
				return (parsingPending());
			}

			if (_line == "\r\n")
			{
				_parsingStep = IN_BODY;
				_line.clear();
				break ;
			}
			else
				ret = parseHeader(_line);
			if (ret != STATUS_NONE)
				return (parsingFailed(ret));
			_line.clear();
		}
	}
	if (_parsingStep == IN_BODY && _vs == NULL)
	{
		std::map<std::string, std::string>::iterator host = _headers.find("host");
		if (host == _headers.end())
			return (parsingFailed(STATUS_BAD_REQUEST));
		ret = associateVirtualServer();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		// A AJOUTER : 
		// if _vs contient une return directive
		//	return parsingSuccedeed();
		ret = associateLocation();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
	}
	if (_parsingStep == IN_BODY)
	{
		if (_contentLength == 0)
			ret = checkIfBody();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
	
		if (_contentLength == 0)
			return (parsingSucceeded());
		else
		{
			std::istringstream	is(buffer);
			unsigned char c;
			while ((is >> c) && (_body.size() + 1 <= _contentLength))
				_body += c;
		}
		if (_body.size() < _contentLength)
		{
			buffer = "";
			return (parsingPending()); //CHECKER SI BAD REQUEST SI > _contentLength
		}
		return (parsingSucceeded());
	}
	return (parsingPending()); // A CHECKER
}

GnlStatus	Request::getNextLine(std::string &buffer)
{
	size_t		strEnd;
	
	buffer = _line + buffer;
	strEnd = buffer.find("\n", 0);
	if (strEnd == std::string::npos)
	{
		_line = buffer;
		buffer = "";
		return (NO_NL);
	}
	if (buffer[strEnd - 1] != '\r')
		return (BAD_REQUEST);
	_line = buffer.substr(0, strEnd + strlen("\r\n"));
	buffer = buffer.substr(strEnd + strlen("\r\n"), std::string::npos);
	return (FOUND_NL);
};

StatusCode	Request::parseRequestLine(std::string requestLine)
{
	// request line empty ?
	std::string	method, protocol, check;
	std::istringstream	is(requestLine);

	if (!(is >> method >> protocol >> _uri) || (is >> check) || _uri[0] != '/')
		return (STATUS_BAD_REQUEST);

	if (method == "GET")
		_method = GET;
	else if (method == "POST")
		_method = POST;
	else if (method == "DELETE")
		_method = DELETE;
	else
		return (STATUS_NOT_IMPLEMENTED);

	if (_uri.size() > MAX_URI_SIZE)
		return (STATUS_URI_TOO_LONG);
	
	// PARSE URI

	if (protocol != PROTOCOL_VERSION)
		return (STATUS_HTTP_VERSION_NOT_SUPPORTED);
		
	return (STATUS_NONE);
}

StatusCode	Request::parseHeader(std::string header)
{
	std::istringstream	is(header);
	std::string	name, value;

	std::getline(is, name, ':');
	std::getline(is, value);
	
	if (name.empty() || value.empty())
		return (STATUS_BAD_REQUEST);

	for_each(name.begin(), name.end(), tolower);
	for_each(value.begin(), value.end(), tolower);

	// DOUBLONS ?

	_headers[name] = value;
	return (STATUS_NONE);
}

StatusCode	Request::checkIfBody()
{
	if (_method == POST)
	{
		std::map<std::string, std::string>::iterator itContentLength = _headers.find("content-length");
		if (itContentLength == _headers.end())
			_contentLength = 0;
		else
		{
			if (itContentLength->second.find_first_not_of("0123456789", 0) != std::string::npos)
				return (STATUS_BAD_REQUEST);
			_contentLength = strtol(itContentLength->second.c_str(), NULL, 10);
			if (_contentLength > _vs->getMaxBodySize())
				return (STATUS_PAYLOAD_TOO_LARGE);
		}
			
	}
	return (STATUS_NONE);
}

ParseRequestResult Request::parsingFailed(StatusCode statusCode)
{
	ParseRequestResult	result;

	result.outcome = REQUEST_FAILURE;
	result.statusCode = statusCode;
	result.location = _location;
	return (result);
}

ParseRequestResult Request::parsingSucceeded()
{
	ParseRequestResult	result;

	result.outcome = REQUEST_SUCCESS;
	result.method = _method;
	result.location = _location;
	return (result);
}

ParseRequestResult Request::parsingPending()
{
	ParseRequestResult	result;

	result.outcome = REQUEST_PENDING;
	return (result);
}

StatusCode	Request::associateVirtualServer()
{
	StatusCode ret;

	fillClientInfos();	
	if (_vsCandidates.size() == 1)
	{
		_vs = _vsCandidates[0];
		return (STATUS_NONE);
	}

	// IP:port check
	std::vector<VirtualServer*> matchingIpPortCombos;
	matchingIpPortCombos = findIpPortMatches();
	if (matchingIpPortCombos.size() == 1)
	{
		_vs = matchingIpPortCombos[0];
		return (STATUS_NONE);
	}

	// server_name check
	ret = extractClientServerName();
	if (ret != STATUS_NONE)
		return (ret);
	if (_hostNameDefined == true)
	{
		VirtualServer *matchingServerName;
		matchingServerName = findServerNamesMatches(matchingIpPortCombos);
		if (matchingServerName != NULL)
		{
			_vs = matchingServerName;
			return (STATUS_NONE);
		}
	}

	// activate default VirtualServer
	for (std::vector<VirtualServer*>::iterator it = _vsCandidates.begin(); it != _vsCandidates.end(); it++)
	{
		if ((*it)->getDefaultVS() == true)
		{
			_vs = *it;
			break ;
		}
	}
	return (STATUS_NONE);
}

void	Request::fillClientInfos()
{
	memset(&_clientAddr, 0, sizeof _clientAddr);
	socklen_t len = sizeof _clientAddr;
	getsockname(_clientfd, (struct sockaddr *) &_clientAddr, &len);

	uint32_t addr = ntohl(_clientAddr.sin_addr.s_addr);
	std::stringstream ss;
	ss << ((addr >> 24) & 0xFF) << "." \
		<< ((addr >> 16) & 0xFF) << "." \
		<< ((addr >> 8) & 0xFF) << "." \
		<< (addr & 0xFF);
	_clientip = ss.str();

	_clientport = ntohs(_clientAddr.sin_port);
	
}

std::vector<VirtualServer*>	Request::findIpPortMatches()
{
	std::vector<VirtualServer*> perfectMatch;
	std::vector<VirtualServer*> generalMatch;

	for (std::vector<VirtualServer*>::iterator it = _vsCandidates.begin(); it != _vsCandidates.end(); it++)
	{
		if ((*it)->getPort() != _clientport)
			_vsCandidates.erase(it);
		if ((*it)->getIP() != _clientip && (*it)->getIP() != "0.0.0.0")
			_vsCandidates.erase(it);
		if ((*it)->getIP() == "0.0.0.0" || ((*it)->getPort() == 8080 && (*it)->getPortByDefault()))
			generalMatch.push_back(*it);
		else
			perfectMatch.push_back(*it);
	}
	if (perfectMatch.empty() == false)
		return (perfectMatch);
	else
		return (generalMatch);
}

StatusCode	Request::extractClientServerName()
{
	_hostName = _headers["host"];
	if (_hostName.find(":", 0) != std::string::npos)
	{
		std::string name(""), port("");
		std::istringstream	iss(_hostName);
		getline(iss, name, ':');
		getline(iss, port);
		if (strtol(port.c_str(), NULL, 10) != _clientport)
			return (STATUS_BAD_REQUEST);
		_hostName = name;
	}
	if (_hostName != _clientip)
		_hostNameDefined = true;
	return (STATUS_NONE);
}

VirtualServer*	Request::findServerNamesMatches(std::vector<VirtualServer*> matchingIpPortCombos)
{
	std::vector<VirtualServer*> leadingWildcard;
	std::vector<VirtualServer*> trailingWildcard;

	for (std::vector<VirtualServer*>::iterator it = matchingIpPortCombos.begin(); it != matchingIpPortCombos.end(); it++)
	{
		// perfect match
		std::string serverName = (*it)->getServerName();
		if (serverName == _hostName)
				return (*it);
		// leading or trailing wildcards : *server_name or server_name*
		if (serverName[0] == '*')
		{
			std::string wServerName = serverName.substr(1, std::string::npos);
			size_t match_pos = _hostName.find(wServerName);
			if (match_pos != std::string::npos && (match_pos - 1 + wServerName.size()) == _hostName.size())
				leadingWildcard.push_back(*it);
		}
		if (leadingWildcard.empty() && serverName[serverName.size() - 1] == '*')
		{
			std::string serverNameW = serverName.substr(0, serverName.size() - 1);
			if (_hostName.substr(0, serverNameW.size()) == serverNameW)
				trailingWildcard.push_back(*it);
		}
	}
	if (leadingWildcard.empty() == false)
	{
		for (std::vector<VirtualServer*>::iterator it = leadingWildcard.begin(); it != leadingWildcard.end(); it++)
		{
			if ((it + 1) != leadingWildcard.end() \
				&& (*it)->getServerName().size() > (*(it + 1))->getServerName().size())
				leadingWildcard.erase(it + 1);
		}
		if (leadingWildcard.size() == 1)
			return (leadingWildcard[0]);
		return (NULL);
	}
	if (trailingWildcard.empty() == false)
	{
		for (std::vector<VirtualServer*>::iterator it = trailingWildcard.begin(); it != trailingWildcard.end(); it++)
		{
			if ((it + 1) != trailingWildcard.end() \
				&& (*it)->getServerName().size() > (*(it + 1))->getServerName().size())
				trailingWildcard.erase(it + 1);
		}
		if (trailingWildcard.size() == 1)
			return (trailingWildcard[0]);
		return (NULL);
	}
	return (NULL);
}

StatusCode	Request::associateLocation()
{
	size_t len(0);

	// exact match
	for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
	{
		if (it->second.getEqualModifier() == true)
		{
			if (it->first == _uri)
			{
				_location = &(it->second);
				break ; // ou RETURN ? REDIRECTION ?
			}	
		}
	}

	// longuest prefix
	if (_location == NULL)
	{
		for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
		{
			if (it->second.getEqualModifier() == false)
			{
				if (_uri.substr(0, it->first.size()) == it->first)
				{
					if (it->first.size() > len)
					{
						_location = &(it->second);
						len = it->first.size();
					}
				}	
			}
		}
	}

	// REDIRECTIONS :
		// index
		// try files
		// rewrite
		// error page

	return (STATUS_NONE);
}