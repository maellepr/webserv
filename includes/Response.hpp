#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "webserv.hpp"

class Response
{
	public:
		Response();
		~Response();

		void			generateResponse(ParseRequestResult &request);
		ResponseOutcome	sendResponseToClient(int fd);
		int				pushStrToClient(int fd, std::string &str);

		void			buildStatusLine();
		void			buildGet(ParseRequestResult &request);

	private:
		std::string							_statusLine;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		StatusCode							_statusCode;

};

#endif