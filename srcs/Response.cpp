#include "../includes/Response.hpp"

Response::Response()
{
}

Response::~Response()
{
}

void	Response::generateResponse(ParseRequestResult &reqRes)
{
	(void) reqRes;
	// if (reqRes.outcome == FAILURE)
	// 	generateErrorPage();
	// if CGI
	//	do CGI
	// Error with no page?
	// redirection ?

	// build response: line + headers (+body)
}