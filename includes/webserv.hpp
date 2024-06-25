#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

void	callException(int errorNum);

class ErrorConnectVS : public std::exception
{
	public:
		virtual const char* what() const throw()
		{
			return ("Error in function connectVirtualServers()");
		}
};