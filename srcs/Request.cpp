#include "../includes/Request.hpp"

Request::Request(int clientfd, std::vector<VirtualServer*> &vsCandidates) : _clientfd(clientfd), _vsCandidates(vsCandidates), _vs(NULL), _parsingStep(IN_REQUESTLINE) {}

Request::~Request() {}

ParseRequestResult	Request::parseBuffer(std::string &buffer)
{
	StatusCode	ret;
	unsigned char c;

	if (_parsingStep == IN_REQUESTLINE)
	{
		// buffer empty ?
		if (getNextLine(buffer) == NO_NL)
		{
			if (_line.size() > MAX_URI_SIZE)
				return (parsingFailed(STATUS_URI_TOO_LONG));
			else
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
			if (_line.size() > MAX_HEADER_SIZE)
				return (parsingFailed(STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE));
			if (getNextLine(buffer) == NO_NL)
				return (parsingPending());

			// if (header.size() > 2 && header.substr(header.size() - 2, std::string::npos) != "\r\n")
			// 	return (parsingFailed(STATUS_BAD_REQUEST));

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
		associateVirtualServer();
	}
	if (_parsingStep == IN_BODY) // A REFAIRE
	{
		ret = checkHeaders();
		if (ret != STATUS_NONE)
			return (parsingFailed(ret));
	
		if (_contentLength == 0) // A CHECK
			return (parsingSucceeded());
		else
		{
			std::istringstream	is(buffer);
			while ((is >> c) && _body.size() < _contentLength)
				_body += c;
		}
		if (_body.size() < _contentLength)
			return (parsingPending()); //CHECKER SI BAD REQUEST SI > _contentLength
		return (parsingSucceeded());
	}
	return (parsingPending()); // A CHECKER
}

GnlStatus	Request::getNextLine(std::string &buffer) // A AMELIORER
{
	size_t		strEnd;
	
	buffer = _line + buffer;
	strEnd = buffer.find("\r\n", 0);
	if (strEnd == std::string::npos)
	{
		_line = buffer;
		buffer = "";
		return (NO_NL);
	}
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
	
	_headers[name] = value;
	return (STATUS_NONE);
}

StatusCode	Request::checkHeaders()
{
	std::map<std::string, std::string>::iterator itHost = _headers.find("Host");
	if (itHost == _headers.end())
		return (STATUS_BAD_REQUEST);
	// find server and location

	if (_method == POST)
	{
		std::map<std::string, std::string>::iterator itContentLength = _headers.find("Content-Length");
		if (itContentLength == _headers.end())
			_contentLength = 0;
		else
		{
			if (itContentLength->second.find_first_not_of("0123456789", 0) != std::string::npos)
				return (STATUS_BAD_REQUEST);
			_contentLength = strtol(itContentLength->second.c_str(), NULL, 10);
			if (_contentLength > _vs->getMaxBodySize()) // A CORRIGER ICI
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
	return (result);
}

ParseRequestResult Request::parsingSucceeded()
{
	ParseRequestResult	result;

	result.outcome = REQUEST_SUCCESS;
	return (result);
}

ParseRequestResult Request::parsingPending()
{
	ParseRequestResult	result;

	result.outcome = REQUEST_PENDING;
	return (result);
}

void	Request::associateVirtualServer()
{
	fillClientInfos();	
	if (_vsCandidates.size() == 1)
	{
		_vs = _vsCandidates[0];
		return ;
	}

	// IP:port check
	std::vector<VirtualServer*> matchingIpPortCombos;
	matchingIpPortCombos = findIpPortMatches();
	if (matchingIpPortCombos.size() == 1)
	{
		_vs = matchingIpPortCombos[0];
		return ;
	}

	// server_name check
	std::vector<VirtualServer*> matchingServerNames;
	matchingServerNames = findServerNamesMatches();

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
		if ((*it)->getIP() == "0.0.0.0")
			generalMatch.push_back(*it);
		else
			perfectMatch.push_back(*it);
	}
	if (perfectMatch.empty() == false)
		return (perfectMatch);
	else
		return (generalMatch);
}

std::vector<VirtualServer*>	Request::findServerNamesMatches()
{

}