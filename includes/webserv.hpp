#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <exception>
# include <vector>
# include <map>
# include <string>
# include <errno.h>
# include <netdb.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/select.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <fcntl.h>
# include <dirent.h>
# include <sstream>
# include <cstdlib>

# define MAX_URI_SIZE 2048
# define MAX_HEADER_SIZE 8192
# define PROTOCOL_VERSION "HTTP/1.1"

// Functions ----------------------------------------------------------------------------- //

void	callException(int errorNum);

// Exceptions ----------------------------------------------------------------------------- //

class ErrorConnectVS : public std::exception
{
	public:
		virtual const char* what() const throw()
		{
			return ("Error in function Server::connectVirtualServers()");
		}
};

class ErrorServerloop : public std::exception
{
	public:
		virtual const char* what() const throw()
		{
			return ("Error in function Server::loop()");
		}
};

class ErrorAcceptNewC : public std::exception
{
	public:
		virtual const char* what() const throw()
		{
			return ("Error in function Server::acceptNewConnection()");
		}
};

// Enums ----------------------------------------------------------------------------- //

typedef enum Method
{
	GET,
	POST,
	DELETE
} Method;

typedef enum RequestOutcome
{
	SUCCESS,
	PENDING,
	FAILURE
} RequestOutcome;

typedef enum StatusCode
{
	STATUS_NONE = 0,
	STATUS_BAD_REQUEST = 404,
	STATUS_NOT_IMPLEMENTED = 501,
	STATUS_PAYLOAD_TOO_LARGE = 413,
	STATUS_URI_TOO_LONG = 414,
	STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
} StatusCode;

// Structures ----------------------------------------------------------------------------- //

typedef struct ParseRequestResult
{	
	RequestOutcome	outcome;
	StatusCode	statusCode;
} ParseRequestResult;

#endif