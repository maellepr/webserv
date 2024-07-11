#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"

class Request
{
	public:
		Request(int clientfd, std::vector<VirtualServer*> &vsCandidates);
		~Request();

		ParseRequestResult			parseBuffer(std::string &buffer);
		GnlStatus					getNextLine(std::string &buffer);
		StatusCode					parseRequestLine(std::string requestLine);
		StatusCode					parseHeader(std::string header);
		StatusCode					checkIfBody();
		ParseRequestResult			parsingFailed(StatusCode statusCode);
		ParseRequestResult			parsingSucceeded();
		ParseRequestResult			parsingPending();
		StatusCode					associateVirtualServer();
		StatusCode					associateLocationRequest();
		void						fillClientInfos();
		std::vector<VirtualServer*>	findIpPortMatches();
		VirtualServer*				findServerNamesMatches(std::vector<VirtualServer*> matchingIpPortCombos);
		StatusCode					extractClientServerName();

	private:
		std::string									_line;
		int											_clientfd;
		struct sockaddr_in							_clientAddr;
		std::string									_clientip;
		int											_clientport;
		std::string									_hostName;
		bool										_hostNameDefined;
		std::vector<VirtualServer*>					_vsCandidates;
		VirtualServer								*_vs;
		Location									*_location;
		Method										_method;
		std::string									_uri;
		std::map<std::string, std::string>			_headers;
		std::string									_body;
		size_t										_contentLength;
		ParsingStep									_parsingStep;

};

#endif