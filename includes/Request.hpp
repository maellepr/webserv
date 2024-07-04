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
		StatusCode					checkHeaders();
		ParseRequestResult			parsingFailed(StatusCode statusCode);
		ParseRequestResult			parsingSucceeded();
		ParseRequestResult			parsingPending();
		void						associateVirtualServer();
		void						fillClientInfos();
		std::vector<VirtualServer*>	findIpPortMatches();
		std::vector<VirtualServer*>	findServerNamesMatches();

	private:
		std::string									_line;
		int											_clientfd;
		struct sockaddr_in							_clientAddr;
		std::string									_clientip;
		int											_clientport;
		std::vector<VirtualServer*>					_vsCandidates;
		VirtualServer								*_vs;
		Method										_method;
		std::string									_uri;
		std::map<std::string, std::string>			_headers;
		std::string									_body;
		size_t										_contentLength;
		ParsingStep									_parsingStep;

};

#endif