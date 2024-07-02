#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "webserv.hpp"

class Response
{
	public:
		Response();
		~Response();

		void	generateResponse(ParseRequestResult &reqRes);

	private:

};

#endif