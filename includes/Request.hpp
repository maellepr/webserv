#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"

class Request
{
	public:
		Request(VirtualServer &vs);
		~Request();

		StatusCode	parseBuffer(std::string buffer);
		StatusCode	parseRequestLine(std::string requestLine);
		StatusCode	parseHeader(std::string header);
		StatusCode	checkHeaders();

	private:
		VirtualServer						_vs;
		Method								_method;
		std::string							_uri;
		std::map<std::string, std::string>	_headers;
		size_t								_contentLength;

};

#endif