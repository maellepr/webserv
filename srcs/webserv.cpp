#include "../includes/webserv.hpp"

void	callException(int errorNum)
{
	if (errorNum == -1)
		throw ErrorConnectVS();
}