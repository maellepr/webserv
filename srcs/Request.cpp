#include "../includes/Request.hpp"

Request::Request()
{
}

Request::~Request()
{
}

StatusCode	Request::parseBuffer(std::string buffer)
{
	std::string	requestLine, header;
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

		if (header.substr(header.size() - 2, npos) != "\r\n")
			return (STATUS_BAD_REQUEST);
		else
			header.resize(header.size() - 2);

		if (header.empty())
			// FIN DES HEADERS
		else
			parseHeader(header);

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