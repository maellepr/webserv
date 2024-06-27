#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "webserv.hpp"

class Request
{
	public:
		Request();
		~Request();

		StatusCode	parseBuffer(std::string buffer);
		StatusCode	parseRequestLine(std::string requestLine);

	private:
		Method		_method;
		std::string	_uri;

};

#endif