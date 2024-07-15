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
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>
# include <dirent.h>
# include <sstream>
# include <fstream>
# include <cstdlib>
# include <algorithm>
# include <ctime>
# include <cstring>

# define DEBUG 1

# define TIMEOUT 10.0

# define MAX_URI_SIZE 2048
# define MAX_HEADER_SIZE 8192
# define PROTOCOL_VERSION "HTTP/1.1"
# define DEFAULT_CONTENT_TYPE "application/octet-stream"

# define RESET	"\e[0m"
# define BOLD	"\e[1m"
# define ITAL	"\e[3m"
# define RED	"\e[31m"
# define ORANGE	"\e[38;2;234;117;26m"
# define LIGHTGREEN	"\e[38;2;105;231;71m"
# define LIGHTBLUE	"\e[36m"
# define PURPLE	"\e[38;2;198;26;234m"
# define GREY	"\e[38;2;100;89;103m"
# define VIOLET "\e[38;2;99;71;231m"


class Location;
class VirtualServer;

# include "VirtualServer.hpp"
# include "Location.hpp"

#define	DEFAULT_MAXBODYSIZE 3145728;
#define KB_IN_BYTES 1024;
#define MB_IN_BYTES 1048576;
#define	GB_IN_BYTES 1073741824;



// Functions ----------------------------------------------------------------------------- //

void		fillStatusMsg();
void		fillContentTypes();
void		callException(int errorNum);
void		extension(const std::string& str, const std::string& extension);
void		isDirectory(const std::string & filename);
bool		isPathADirectory(const std::string &path);
bool		isPathADRegularFile(const std::string &path);
bool		isUriValid(const std::string &uri);
bool		readContent(std::string &uri, std::string &content);

// Exceptions ----------------------------------------------------------------------------- //

class ErrorConfigFile : public std::exception
{
	public:
		ErrorConfigFile(std::string errorMsg) throw();
		~ErrorConfigFile() throw();

		virtual const char* what() const throw();
		// std::string	errorMsg;
		
	private:
		std::string	_errorMsg;
};

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
	REQUEST_SUCCESS,
	REQUEST_PENDING,
	REQUEST_FAILURE
} RequestOutcome;

typedef enum ResponseOutcome
{
	RESPONSE_SUCCESS,
	RESPONSE_PENDING,
	RESPONSE_FAILURE
} ResponseOutcome;

typedef enum ParsingStep
{
	IN_REQUESTLINE,
	IN_HEADERS,
	IN_BODY,
	DONE
} ParsingStep;

typedef enum GnlStatus
{
	FOUND_NL,
	NO_NL,
	BAD_REQUEST
} GnlStatus;

typedef enum StatusCode
{
	STATUS_NONE = 0,
	STATUS_OK = 200,
	STATUS_MOVED_PERMANENTLY = 301,
	STATUS_FORBIDDEN = 403,
	STATUS_BAD_REQUEST = 400,
	STATUS_NOT_FOUND = 404,
	STATUS_REQUEST_TIMEOUT = 408,
	STATUS_PAYLOAD_TOO_LARGE = 413,
	STATUS_URI_TOO_LONG = 414,
	STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	STATUS_INTERNAL_SERVER_ERROR = 500,
	STATUS_NOT_IMPLEMENTED = 501,
	STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
} StatusCode;

// Structures ----------------------------------------------------------------------------- //

typedef struct ParseRequestResult
{	
	RequestOutcome	outcome;		// parsing result : SUCCES PENDING FAILURE
	StatusCode		statusCode;		// SUCCES -> code: none, FAILURE -> code: 1-500
	Method			method;			// GET POST DELETE
	std::string		uri;
	std::string		hostName;
	VirtualServer	*vs;
	Location		*location;		// location qui a matche a la config
} ParseRequestResult;

#endif