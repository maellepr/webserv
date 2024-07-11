#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserv.hpp"
# include "Location.hpp"

class Response
{
	public:
		Response();
		~Response();

		void			fillStatusMsg();
		void			generateResponse(ParseRequestResult &request);
		ResponseOutcome	sendResponseToClient(int fd);
		int				pushStrToClient(int fd, std::string &str);

		void			buildStatusLine();
		void			buildErrorPage(ParseRequestResult &request);
		void			buildGet(ParseRequestResult &request);
		void			buildPage();
		void			buildAutoindexPage();
		void			associateLocationResponse(ParseRequestResult &request);

	private:
		std::string											_statusLine;
		std::map<std::string, std::string>					_headers;
		std::string											_body;
		StatusCode											_statusCode;
		std::map<StatusCode, std::string>					_statusMsg;
		std::string											_finalURI;
		std::string											_rootDir;
		std::map<std::string, std::vector<std::string> >	_configLocation;

};

#endif