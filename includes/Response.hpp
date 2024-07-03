#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "webserv.hpp"

class Response
{
	public:
		Response();
		~Response();

		void			generateResponse(ParseRequestResult &reqRes);
		ResponseOutcome	sendResponseToClient(int fd);
		int				pushStrToClient(int fd, std::string &str);

	private:
		std::string				_responseLine;
		std::map<std::string, std::string>	_headers;
		std::string				_body;
};

#endif