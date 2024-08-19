#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserv.hpp"
# include "Location.hpp"

extern std::map<StatusCode, std::string>	STATUS_MESSAGES;
extern std::map<std::string, std::string>	CONTENT_TYPES;

class Response
{
	public:
		Response();
		~Response();

		void						fillStatusMsg();
		void						generateResponse(ParseRequestResult &request);
		ResponseOutcome				sendResponseToClient(int fd);
		int							pushStrToClient(int fd, std::string &str);

		void						buildCgi(ParseRequestResult &request);
		std::vector<std::string>	doEnvCgi(ParseRequestResult &request);
		void						exportToEnv(std::vector<std::string> &env, const std::string &key, const std::string &value);
		std::string					readResponse(int fd);
		void						buildStatusLine();
		void						buildErrorPage(ParseRequestResult &request, StatusCode statusCode);
		void						buildGet(ParseRequestResult &request);
		void						buildPost(ParseRequestResult &request);
		void						buildPage(ParseRequestResult &request);
		std::vector<std::string>	doDirListing(DIR *dir);
		void						buildAutoindexPage(ParseRequestResult &request);
		Location					*associateLocationResponse(ParseRequestResult &request, std::string index);
		void						listUploadFiles(ParseRequestResult &request);

	private:
		std::string											_statusLine;
		std::map<std::string, std::string>					_headers;
		std::string											_body;
		StatusCode											_statusCode;
		std::string											_finalURI;
		std::string											_rootDir;
		std::map<std::string, std::vector<std::string> >	_configLocation;
		std::map<std::string, std::string> 					_uploads;
		

};

#endif