#include "../includes/Request.hpp"

Request::Request(int clientfd, std::vector<VirtualServer*> &vsCandidates) : _clientfd(clientfd), _hostNameDefined(false), _vsCandidates(vsCandidates), _vs(NULL), _location(NULL), _contentLength(0), _parsingStep(IN_REQUESTLINE) {}

Request::~Request() {}

ParseRequestResult	Request::parseBuffer(std::string &buffer)
{
	// std::cout << LIGHTBLUE << "PARSE_BUFFER START" << RESET << std::endl;
	StatusCode	ret;
	GnlStatus	gnl;

	if (_parsingStep == IN_REQUESTLINE)
	{
		// std::cout << LIGHTBLUE << "PARSE_BUFFER 1" << RESET << std::endl;
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
		// std::cout << "method = " << _method << std::endl;
		// std::cout << "URI = " << _uri << std::endl;
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
			// std::cout << PURPLE << "_line = " << _line << RESET << std::endl;

			if (_line.empty())
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
		// std::cout << GREY << "<header : value>" << RESET << std::endl;
		// for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		// {
		// 	std::cout << GREY << it->first << " : " 
		// 		 << GREY << it->second << RESET << std::endl;
		// }
		std::map<std::string, std::string>::iterator host = _headers.find("host");
		if (host == _headers.end())
			return (parsingFailed(STATUS_BAD_REQUEST));
		ret = associateVirtualServer();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		std::cout << PURPLE << "Chosen server infos : " << RESET << std::endl;
		std::cout << PURPLE << "IP and port : " << _vs->getIP() << ":" << _vs->getPort() << RESET << std::endl;
		std::cout << PURPLE << "Server_name : " << _vs->getServerName() << RESET << std::endl;
		ret = associateLocationRequest();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		std::cout << GREY << "Chosen location infos : " << RESET << std::endl;
		std::cout << GREY << "Location prefix : " << _location->getPrefix() << RESET << std::endl;
		std::cout << GREY << "Location root : " << _location->getConfigLocation()["rootDir"][0] << RESET << std::endl;
		std::cout << GREY << "Server acting as location (0: false, 1:true) : " << _location->getServerActAsLocation() << RESET << std::endl;
		// for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
		// {
		// 	if (&it->second == _location)
		// 	{
		// 		std::cout << GREY << "Location prefix : " << it->first << RESET << std::endl;
		// 		break ;
		// 	}
		// }
	}
	// std::cout << LIGHTBLUE << "AFTER ASSOCIATE SERVER" << RESET << std::endl;
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
			buffer = "";
		}
		if (_body.size() < _contentLength)
			return (parsingPending()); //CHECKER SI BAD REQUEST SI > _contentLength
		return (parsingSucceeded());
	}
	return (parsingPending()); // A CHECKER
	// std::cout << LIGHTBLUE << "PARSE_BUFFER END" << RESET << std::endl;
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
	_line = buffer.substr(0, strEnd - 1);
	buffer = buffer.substr(strEnd + 1, std::string::npos);
	// std::cout << PURPLE << "_line = " << _line << RESET << std::endl;
	// std::cout << PURPLE << "buffer = " << buffer << RESET << std::endl;
	return (FOUND_NL);
};

StatusCode	Request::parseRequestLine(std::string requestLine)
{
	// request line empty ?
	std::string	method, protocol, check;
	std::istringstream	is(requestLine);

	// std::cout << LIGHTBLUE << "requestline = " << requestLine << RESET << std::endl;

	if (!(is >> method >> _uri >> protocol) || (is >> check) || _uri[0] != '/')
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

	if (protocol != PROTOCOL_VERSION)
		return (STATUS_HTTP_VERSION_NOT_SUPPORTED);
		
	return (STATUS_NONE);
}

StatusCode	Request::parseHeader(std::string header)
{
	std::istringstream	is(header);
	std::string	name, value;

	std::getline(is, name, ':');
	is.ignore();
	std::getline(is, value);
	
	if (name.empty() || value.empty())
		return (STATUS_BAD_REQUEST);

	for (std::string::iterator it = name.begin(); it != name.end(); it++)
	{
		(*it) = tolower(*it);
	}

	for (std::string::iterator it = value.begin(); it != value.end(); it++)
	{
		(*it) = tolower(*it);
	}

	// for_each(name.begin(), name.end(), tolower);
	// for_each(value.begin(), value.end(), tolower);

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
	result.uri = _uri;
	result.hostName = _hostName;
	result.location = _location;
	result.vs = _vs;
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
	// std::cout << LIGHTBLUE << "associateVirtualServer 1" << RESET << std::endl;
	if (_vsCandidates.size() == 1)
	{
		_vs = _vsCandidates[0];
		return (STATUS_NONE);
	}
	// std::cout << LIGHTBLUE << "associateVirtualServer 2" << RESET << std::endl;

	// IP:port check
	std::vector<VirtualServer*> matchingIpPortCombos;
	matchingIpPortCombos = findIpPortMatches();
	if (matchingIpPortCombos.size() == 1)
	{
		_vs = matchingIpPortCombos[0];
		return (STATUS_NONE);
	}
	// std::cout << LIGHTBLUE << "associateVirtualServer 3" << RESET << std::endl;

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
	std::cout << LIGHTBLUE << "associateVirtualServer 4" << RESET << std::endl;

	// activate default VirtualServer
	for (std::vector<VirtualServer*>::iterator it = _vsCandidates.begin(); it != _vsCandidates.end(); it++)
	{
		if ((*it)->getDefaultVS() == true)
		{
			_vs = *it;
			break ;
		}
	}
	std::cout << LIGHTBLUE << "associateVirtualServer 5" << RESET << std::endl;
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
			continue;
		if ((*it)->getIP() != _clientip && (*it)->getIP() != "0.0.0.0")
			continue;
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
			if (match_pos != std::string::npos && (match_pos + wServerName.size()) == _hostName.size())
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

StatusCode	Request::associateLocationRequest()
{
	// no location available => server acts as location
	if (_vs->getLocations().empty() && _vs->getLocationsEqual().empty())
	{
		Location	location(_vs->getReturnPages(), *_vs, true);
		location.setPrefix("/");
		_vs->getLocations()["/"] = location;
		_location = &_vs->getLocations()["/"];
		return (STATUS_NONE);
	}

	// for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
	// {
	// 	std::cout << RED << "equal it = <" << it->first << ">" << RESET << std::endl;
	// 	std::cout << RED << "equalmodif = " << it->second.getEqualModifier() << RESET << std::endl;
	// }

	// exact match
	for (std::map<std::string, Location>::iterator it = _vs->getLocationsEqual().begin(); it != _vs->getLocationsEqual().end(); it++)
	{
		// if (it->second.getEqualModifier() == true)
		// {
		if (it->first == _uri)
		{
			_location = &(it->second);
			return (STATUS_NONE);
		}	
		// }
	}

	// longuest prefix
	size_t len(0);
	for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
	{
		// if (it->second.getEqualModifier() == false)
		// {
		if (_uri.substr(0, it->first.size()) == it->first)
		{
			if (it->first.size() > len)
			{
				_location = &(it->second);
				len = it->first.size();
			}
		}	
		// }
	}

	// no location available => server acts as location
	if (_location == NULL)
	{
		Location	location(_vs->getReturnPages(), *_vs, true);
		location.setPrefix("/");
		_vs->getLocations()["/"] = location;
		_location = &_vs->getLocations()["/"];
	}

	return (STATUS_NONE);

	// REDIRECTIONS ??
		// index : OK
		// try files
		// rewrite
		// error page : OK
}