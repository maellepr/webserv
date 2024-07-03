#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"

class Request
{
	public:
		Request();
		~Request();

		ParseRequestResult	parseBuffer(std::string &buffer);
		GnlStatus			getNextLine(std::string &buffer);
		StatusCode			parseRequestLine(std::string requestLine);
		StatusCode			parseHeader(std::string header);
		StatusCode			checkHeaders();
		ParseRequestResult	parsingFailed(StatusCode statusCode);
		ParseRequestResult	parsingSucceeded();
		ParseRequestResult	parsingPending();

	private:
		std::string							_line;
		VirtualServer						_vs;
		Method								_method;
		std::string							_uri;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		size_t								_contentLength;
		ParsingStep							_parsingStep;

};

#endif