#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <exception>
#include <vector>
#include <map>
#include <string>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

void	callException(int errorNum);


void	extension(const std::string& str, const std::string& extension);
void	isDirectory(const std::string & filename);

class ErrorConfigFile : public std::exception
{
	public:
		ErrorConfigFile(std::string errorMsg) throw();
		~ErrorConfigFile() throw();

		virtual const char* what() const throw();
		std::string	errorMsg;
		
	private:
		// std::string	_errorMsg;
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

#endif