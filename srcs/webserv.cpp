#include "../includes/webserv.hpp"

void	callException(int errorNum)
{
	if (errorNum == -1)
		throw ErrorConnectVS();
	if (errorNum == -2)
		throw ErrorServerloop();
	if (errorNum == -3)
		throw ErrorAcceptNewC();
}

ErrorConfigFile::ErrorConfigFile(std::string msg) throw() : _errorMsg(msg) {}
ErrorConfigFile::~ErrorConfigFile() throw() {}

const char *ErrorConfigFile::what() const throw()
{
	return _errorMsg.c_str();
}

ErrorResponse::ErrorResponse(std::string msg) throw() : _errorMsg(msg) {}
ErrorResponse::~ErrorResponse() throw() {}

const char *ErrorResponse::what() const throw()
{
	return _errorMsg.c_str();
}