#include "../includes/Request.hpp"

Request::Request(VirtualServer &vs) : _vs(vs)
{
}

Request::~Request()
{
}

StatusCode	Request::parseBuffer(std::string buffer)
{
	std::string	requestLine, header, body;
	std::istringstream	is(buffer);
	StatusCode	ret;

	std::getline(is, requestLine);
	if (is.fail())
		return (STATUS_STOP); //A CHECKER
	
	ret = parseRequestLine(requestLine);
	if (ret != STATUS_NONE)
		return (ret);

	while (getline(is, header))
	{
		if (is.fail())
			return (STATUS_STOP); //A CHECKER
		if (header.size() > MAX_HEADER_SIZE)
			return (STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE);

		if (header.size() > 2 && header.substr(header.size() - 2, std::string::npos) != "\r\n")
			return (STATUS_BAD_REQUEST);

		if (header == "\r\n")
			break ;
		else
			ret = parseHeader(header);
		if (ret != STATUS_NONE)
			return (ret);
	}

	if (header == "\r\n")
	{
		ret = checkHeaders();
		if (ret != STATUS_NONE)
			return (ret);
	}
	
	getline(is, body);
	{
		
	}
	// parse Message body

	return (STATUS_NONE);
}

StatusCode	Request::parseRequestLine(std::string requestLine)
{
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
			if (_contentLength > _vs.getMaxBodySize()) // A CORRIGER ICI
				return (STATUS_PAYLOAD_TOO_LARGE);
		}
			
	}
	return (STATUS_NONE);
}