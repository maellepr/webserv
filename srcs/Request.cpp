#include "../includes/Request.hpp"

Request::Request(int clientfd, std::vector<VirtualServer*> &vsCandidates) : _clientfd(clientfd), _hostNameDefined(false), _vsCandidates(vsCandidates), _vs(NULL), _location(NULL), _contentLength(0), _parsingStep(IN_REQUESTLINE), _isUpload(false), _isChunked(false), _keepAlive(true), _chunkedLen(0), _chunkStep(IN_LEN) {}

Request::~Request() {}

ParseRequestResult	Request::parseBuffer(std::string &buffer)
{
	StatusCode	ret(STATUS_NONE);
	GnlStatus	gnl;

	// if (REQ_COM)
	// 	std::cerr << LIGHTBLUE << "PARSE_BUFFER" << RESET << std::endl;
	if (_parsingStep == IN_REQUESTLINE)
	{
		// if (REQ_COM)
		// 	std::cerr << LIGHTBLUE << "REQUESTLINE" << RESET << std::endl;
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
		// if (REQ_COM)
		// 	std::cerr << LIGHTBLUE << "HEADERS" << RESET << std::endl;
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
		// if (REQ_COM)
		// 	std::cerr << LIGHTBLUE << "BODY => vs" << RESET << std::endl;
		std::map<std::string, std::string>::iterator host = _headers.find("host");
		if (host == _headers.end())
			return (parsingFailed(STATUS_BAD_REQUEST));
		ret = associateVirtualServer();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		// if (REQ_COM)
		// {
		// 	std::cerr << PURPLE << "Chosen server infos : " << RESET << std::endl;
		// 	std::cerr << PURPLE << "IP and port : " << _vs->getIP() << ":" << _vs->getPort() << RESET << std::endl;
		// 	std::cerr << PURPLE << "Server_name : " << _vs->getServerName() << RESET << std::endl;
		// }
		ret = associateLocationRequest();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		// if (REQ_COM)
		// {
		// 	std::cerr << GREY << "Chosen location infos : " << RESET << std::endl;
		// 	std::cerr << GREY << "Location prefix : " << _location->getPrefix() << RESET << std::endl;
		// 	std::cerr << GREY << "Location root : " << _location->getConfigLocation()["rootDir"][0] << RESET << std::endl;
		// 	std::cerr << GREY << "Server acting as location (0: false, 1:true) : " << _location->getServerActAsLocation() << RESET << std::endl;
		// }
	}
	if (_parsingStep == IN_BODY)
	{
		// if (REQ_COM)
			// std::cerr << LIGHTBLUE << "BODY" << RESET << std::endl;
		if (_contentLength == 0)
			ret = checkIfBody();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
		if (_contentLength == 0 && _isChunked == false)
		{
			if (buffer.empty() == false)
				return (parsingFailed(STATUS_BAD_REQUEST));
			return (parsingSucceeded());
		}
		else
		{
			if (_isUpload)
			{
				// if (REQ_COM)
					// std::cerr << LIGHTBLUE << "PARSE UPLOAD" << RESET << std::endl;
				_isUpload = false;
				std::vector<unsigned char> v(buffer.begin(), buffer.end());
				buffer = "";
				for (std::vector<unsigned char>::iterator vit = v.begin(); vit != v.end(); vit++)
				{
					// std::cerr << LIGHTBLUE << "char = ";
					// if (*vit == '\r')
					// 	std::cerr << "CR" << RESET << std::endl;
					// else if (*vit == '\n')
					// 	std::cerr << "LF" << RESET << std::endl;
					// else
					// 	std::cerr << *vit << RESET << std::endl;
					_ucharLine.push_back(*vit);
					_ucharBody.push_back(*vit);
					if (*vit == '\n')
					{
						// std::cerr << LIGHTBLUE << "BLACKSLASH" << RESET << std::endl;
						// std::cerr << RED << "_ucharLine.size() = " << _ucharLine.size() << RESET << std::endl;
						// std::cerr << RED << "_ucharLine[_ucharLine.size() - 1] = ";
						// if (_ucharLine[_ucharLine.size() - 2] == '\r')
						// 	std::cerr << "CR" << RESET << std::endl;
						// else if (_ucharLine[_ucharLine.size() - 2] == '\n')
						// 	std::cerr << "LF" << RESET << std::endl;
						// else
						// 	std::cerr << _ucharLine[_ucharLine.size() - 2] << RESET << std::endl;
						if (_ucharLine.size() > 1 && _ucharLine[_ucharLine.size() - 2] == '\r')
						{
							// std::cerr << LIGHTBLUE << "GO OUT" << RESET << std::endl;
							// std::cerr << RED << "_ucharLine = " << stringifyVector(_ucharLine) << RESET << std::endl;
							// std::cerr << RED << "_boundary = " << _boundary + "--\r\n" << RESET << std::endl;
							if (stringifyVector(_ucharLine) == _boundary + "--\r\n")
							{
								// std::cerr << LIGHTBLUE << "STRINGYFY" << RESET << std::endl;
								_isUpload = true;
								_ucharLine.clear();
								// std::cerr << RED << "_ucharBody.size() = " << _ucharBody.size() << RESET << std::endl;
								// std::cerr << RED << "v.size() = " << v.size() << RESET << std::endl;
								// std::cerr << RED << "_contentLength = " << _contentLength << RESET << std::endl;
								if (_ucharBody.size() != _contentLength)
									return (parsingFailed(STATUS_BAD_REQUEST));
								break;
							}
						}
						_ucharLine.clear();
					}
				}
				if (_isUpload == false)
				{
					_isUpload = true;
					return (parsingPending());
				}
				_body = stringifyVector(_ucharBody);
				return (parsingSucceeded());
			}
			else if (_isChunked)
			{
				// if (REQ_COM)
				// 	std::cerr << LIGHTBLUE << "PARSE CHUNK" << RESET << std::endl;
				std::vector<unsigned char> v(buffer.begin(), buffer.end());
				buffer = "";
				for (std::vector<unsigned char>::iterator vit = v.begin(); vit != v.end(); vit++)
				{
					if (_chunkStep == IN_LEN)
					{
						std::size_t power(1);
						_ucharLine.push_back(*vit);
						if (*vit == '\n')
						{
							if (_ucharLine.size() < 2 || _ucharLine[_ucharLine.size() - 2] != '\r')
								return (parsingFailed(STATUS_BAD_REQUEST));
							_ucharLine.pop_back();
							_ucharLine.pop_back();
							while (_ucharLine.empty() == false)
							{
								// if (_ucharLine.back() == '\r')
								// 	std::cerr << PURPLE << "_ucharLine.back() = " << "CR" << RESET << std::endl;
								// else if (_ucharLine.back() == '\n')
								// 	std::cerr << PURPLE << "_ucharLine.back() = " << "LF" << RESET << std::endl;
								// else
								// 	std::cerr << PURPLE << "_ucharLine.back() = " << _ucharLine.back() << RESET << std::endl;
								if (HEXA_BASE.find(toupper(_ucharLine.back())) == HEXA_BASE.end())
									return (parsingFailed(STATUS_BAD_REQUEST));
								_chunkedLen += HEXA_BASE[toupper(_ucharLine.back())] * power;
								_ucharLine.pop_back();
								power *= 16;
							}
							if (_chunkedLen > _vs->getMaxBodySize() || _chunkedLen + _contentLength > _vs->getMaxBodySize())
								return (parsingFailed(STATUS_PAYLOAD_TOO_LARGE));
							//std::cerr << PURPLE << "_chunkedLen = " << _chunkedLen << RESET << std::endl;
							if (_chunkedLen == 0)
							{
								// _isChunked = true;
								_ucharLine.clear();
								// if (_ucharBody.size() != _contentLength)
								// if (*vit == '\r')
								// 	std::cerr << PURPLE << "*vit = " << "CR" << RESET << std::endl;
								// else if (*vit == '\n')
								// 	std::cerr << PURPLE << "*vit = " << "LF" << RESET << std::endl;
								// else
								// 	std::cerr << PURPLE << "*vit = " << *vit << RESET << std::endl;
								vit++;
								if (vit == v.end() || (vit + 1) == v.end() || (vit + 2) == v.end())
									return (parsingFailed(STATUS_BAD_REQUEST));
								if (*vit != '\r' || *(++vit) != '\n' || ++vit != v.end())
									return (parsingFailed(STATUS_BAD_REQUEST));
								_ucharBody.push_back('\r');
								_ucharBody.push_back('\n');
								_contentLength += 2; // ??
								_body = stringifyVector(_ucharBody);
								//std::cerr << VIOLET << "body = " << _body << RESET << std::endl;
								return (parsingSucceeded());
							}
							_chunkStep = IN_CHUNK;
						}
					}
					else
					{
						_contentLength += _chunkedLen;
						while (_chunkedLen && vit != v.end())
						{
								// if (*vit == '\r')
								// 	std::cerr << PURPLE << "*vit = " << "CR" << RESET << std::endl;
								// else if (*vit == '\n')
								// 	std::cerr << PURPLE << "*vit = " << "LF" << RESET << std::endl;
								// else
								// 	std::cerr << PURPLE << "*vit = " << *vit << RESET << std::endl;
							_ucharLine.push_back(*vit);
							_ucharBody.push_back(*vit);
							_chunkedLen--;
							vit++;
						}
						if (_chunkedLen != 0 || vit == v.end() || *vit != '\r' || *(++vit) != '\n')
							return (parsingPending());
						_ucharLine.clear();
						_chunkStep = IN_LEN;
					}
				}
			}
			else
			{
				while (buffer.empty() == false && (_body.size() + 1 <= _contentLength))
				{
					_body.push_back(buffer[0]);
					buffer = buffer.substr(1, std::string::npos);
				}
				// std::cerr << DARKBLUE << "_body.size() = " << _body.size() << std::endl;
				// std::cerr << DARKBLUE << "_contentLength = " << _contentLength << std::endl;
				// std::cerr << DARKBLUE << "_BODY = " << _body << RESET << std::endl;
				if (_body.size() < _contentLength)
				{
					buffer = "";
					return (parsingPending());
				}
				if (buffer.empty() == false)
					return (parsingFailed(STATUS_BAD_REQUEST));
				return (parsingSucceeded());
			}
		}
	}
	return (parsingPending());
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
	return (FOUND_NL);
};

StatusCode	Request::parseRequestLine(std::string requestLine)
{
	std::string	method, protocol, check;
	std::istringstream	is(requestLine);

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

	if (_method == GET)
	{
		size_t queryBegin = _uri.find('?');
		if (queryBegin != std::string::npos)
		{
			_query = _uri.substr(queryBegin + 1);
			_uri = _uri.substr(0, queryBegin);
		}
	}
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

	// doublons => bad request
	if (_headers.find(name) != _headers.end())
		return (STATUS_BAD_REQUEST);
	
	// connexion settings if any
	if (name == "connexion")
	{
		// if (REQ_COM)
		// {
		// 	for (std::string::iterator it = value.begin(); it != value.end(); it++)
		// 	{
		// 		if (*it == '\n')
		// 			std::cerr << "LF\n";
		// 		else if (*it == '\r')
		// 			std::cerr << "CR\n";
		// 		else
		// 			std::cerr << *it;
		// 	}
		// }
		if (value == "close")
			_keepAlive = false;
		else if (value != "keep-alive")
			return (STATUS_BAD_REQUEST);
	}

	_headers[name] = value;
	return (STATUS_NONE);
}

StatusCode	Request::checkIfBody()
{
	// if (REQ_COM)
	// 	std::cerr << "Check if body\n";
	_contentLength = 0;
	std::map<std::string, std::string>::iterator it = _headers.find("content-length");
	if (it != _headers.end())
	{
		if (it->second.find_first_not_of("0123456789", 0) != std::string::npos)
			return (STATUS_BAD_REQUEST);
		_contentLength = strtol(it->second.c_str(), NULL, 10);
		if (_contentLength > _vs->getMaxBodySize())
			return (STATUS_PAYLOAD_TOO_LARGE);
		std::map<std::string, std::string>::iterator it = _headers.find("content-type");
		if (it == _headers.end())
			return (STATUS_BAD_REQUEST);
		else
		{
			if (it->second.substr(0, strlen("text/plain")) == "text/plain")
				return (STATUS_UNSUPPORTED_MEDIA_TYPE);
			else if (it->second.substr(0, strlen("multipart/form-data")) == "multipart/form-data")
			{
				_isUpload = true;
				size_t pos = it->second.find("boundary=");
				if (pos == std::string::npos)
					return (STATUS_BAD_REQUEST);
				else
					_boundary = "--" + it->second.substr(pos + 9);
			}
		}
	}
	std::map<std::string, std::string>::iterator ita = _headers.find("transfer-encoding");
	if (ita != _headers.end() && _contentLength != 0)
		return (STATUS_BAD_REQUEST);
	if (ita != _headers.end())
	{
		if (ita->second != "chunked")
			return (STATUS_BAD_REQUEST);
		else
			_isChunked = true;
	}
	return (STATUS_NONE);
}

void	Request::fillParseRequestResult(ParseRequestResult &result)
{
	result.statusCode = STATUS_NONE;
	result.method = _method;
	result.uri = _uri;
	result.query = _query;
	result.body = _body;
	result.contentLenght = _contentLength;
	result.hostName = _hostName;
	result.location = _location;
	result.vs = _vs;
	result.isUpload = _isUpload;
	if (_isUpload)
		result.boundary = _boundary;
	result.keepAlive = _keepAlive;
}

ParseRequestResult Request::parsingFailed(StatusCode statusCode)
{
	// if (REQ_COM)
	// 	std::cerr << "PARSING FAILED\n";
	ParseRequestResult	result;

	fillParseRequestResult(result);
	result.outcome = REQUEST_FAILURE;
	result.statusCode = statusCode;
	return (result);
}

ParseRequestResult Request::parsingSucceeded()
{
	// if (REQ_COM)
	// 	std::cerr << "PARSING SUCCEEDED\n";
	ParseRequestResult	result;

	fillParseRequestResult(result);
	result.outcome = REQUEST_SUCCESS;

	//peut etre a bouger de place (dans fillParseRequestResult)
	for(std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		result.headers[it->first] = it->second;
	result.contentLenght = _contentLength;

	return (result);
}

ParseRequestResult Request::parsingPending()
{
	// if (REQ_COM)
	// 	std::cerr << "PARSING PENDING\n";
	ParseRequestResult	result;

	fillParseRequestResult(result);
	result.outcome = REQUEST_PENDING;
	return (result);
}

StatusCode	Request::associateVirtualServer()
{
	StatusCode ret;

	fillClientInfos();
	ret = extractClientServerName();
	if (ret != STATUS_NONE)
		return (ret);

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
			return (STATUS_NONE);
		}
	}
	return (STATUS_INTERNAL_SERVER_ERROR);
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
		if (count(serverName.begin(), serverName.end(), '*') != 1)
			continue;
		// leading or trailing wildcards : *server_name or server_name*
		if (serverName[0] == '*')
		{
			std::string wServerName = serverName.substr(1, std::string::npos);
			size_t match_pos = _hostName.find(wServerName);
			if (match_pos != std::string::npos && (match_pos + wServerName.size()) == _hostName.size())
			{
				leadingWildcard.push_back(*it);
			}
		}
		if (leadingWildcard.empty() && serverName[serverName.size() - 1] == '*')
		{
			std::string serverNameW = serverName.substr(0, serverName.size() - 1);
			if (_hostName.substr(0, serverNameW.size()) == serverNameW)
			{
				trailingWildcard.push_back(*it);
			}
		}
	}
	if (leadingWildcard.empty() == false)
	{
		VirtualServer *leadvs = NULL;
		size_t len = 0;
		for (std::vector<VirtualServer*>::iterator it = leadingWildcard.begin(); it != leadingWildcard.end(); it++)
		{
			if (len < (*it)->getServerName().size())
			{
				leadvs = (*it);
				len = (*it)->getServerName().size();
			}
		}
		return (leadvs);
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

	// exact match
	for (std::map<std::string, Location>::iterator it = _vs->getLocationsEqual().begin(); it != _vs->getLocationsEqual().end(); it++)
	{
		if (it->first == _uri)
		{
			_location = &(it->second);
			return (STATUS_NONE);
		}	
	}

	// longuest prefix
	size_t len(0);
	for (std::map<std::string, Location>::iterator it = _vs->getLocations().begin(); it != _vs->getLocations().end(); it++)
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

	// no location available => server acts as location
	if (_location == NULL)
	{
		Location	location(_vs->getReturnPages(), *_vs, true);
		location.setPrefix("/");
		_vs->getLocations()["/"] = location;
		_location = &_vs->getLocations()["/"];
	}

	return (STATUS_NONE);
}