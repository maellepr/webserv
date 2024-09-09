#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserv.hpp"
# include "Location.hpp"
# include "Client.hpp"

class Client;

extern std::map<StatusCode, std::string>	STATUS_MESSAGES;
extern std::map<std::string, std::string>	CONTENT_TYPES;

class Response
{
	public:
		Response();
		~Response();

		void						setFdInfos(int fdMax, fd_set writeFds, fd_set readFds);
		// void						setClient(std::vector<int> &c);
		void						setClient(std::map<int, Client> *c);

		void						setSocketBoundVs(std::map<int, std::vector<VirtualServer*> > &vs);
		void						fillStatusMsg();
		void						generateResponse(ParseRequestResult &request);
		ResponseOutcome				sendResponseToClient(int fd);
		int							pushStrToClient(int fd, std::string &str);
		bool						loopDetectedReturn(ParseRequestResult &request);
		void						buildReturn(ParseRequestResult &request);
		bool						methodIsAuthorize(ParseRequestResult	&request);
		void						buildDelete(ParseRequestResult &request);
		void						buildCgi(ParseRequestResult &request);
		std::string					findCgi();
		void						closeAllFd();
		void						initCgi(ParseRequestResult &request);
		void						buildPageCgi();
		int							getLine(std::string &buffer);
		std::vector<std::string>	doEnvCgi(ParseRequestResult &request);
		void						exportToEnv(std::vector<std::string> &env, const std::string &key, const std::string &value);
		std::string					readResponse(int fd);
		void						buildStatusLine();
		void						buildErrorPage(ParseRequestResult &request, StatusCode statusCode);
		void						buildGet(ParseRequestResult &request);
		void						buildPost(ParseRequestResult &request);
		void						buildPage(ParseRequestResult &request);
		std::vector<std::string>	doDirListing(DIR *dir);
		void						buildResponseJs(ParseRequestResult &request);
		void						buildAutoindexPage(ParseRequestResult &request);
		Location					*associateLocationResponse(ParseRequestResult &request, std::string index);
		void						listUploadFiles(ParseRequestResult &request);
		bool						getErrorCloseSocket();

	private:
		std::string											_statusLine;
		std::map<std::string, std::string>					_headers;
		std::map<std::string, std::string>					_cookies;
		std::string											_body;
		StatusCode											_statusCode;
		std::string											_finalURI;
		char												*_finalUriChar;
		char												*_cgi;
		std::string											_returnRes;
		std::string											_rootDir;
		std::map<std::string, std::vector<std::string> >	_configLocation;
		std::map<std::string, std::string> 					_uploads;
		bool												_errorCloseSocket;
		int													_fd_max;
		fd_set												_write_fds;
		fd_set												_read_fds;
	
		// std::vector<int>									_clients;

		std::map<int, Client>								*_c;
		std::map<int, std::vector<VirtualServer*> >			_socketBoundVs;

};

#endif