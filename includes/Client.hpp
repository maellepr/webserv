#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Client 
{
    public :
        Client();
        ~Client();

        void			setFd(int fd);
        int    			getFd();
		void			setConnectedServers(int serverfd, std::map<int, std::vector<VirtualServer*> >	&socketBoundVs);
		// VirtualServer	&getConnectedServer();
		// void			setMaxBodySize(size_t maxBodySize);
		// size_t			getMaxBodySize();

        int readRequest();
        ResponseOutcome writeResponse();
        
    private :
        int 						_clientfd;
		std::vector<VirtualServer*>	_vsCandidates;
		Request						*_request;
		Response					*_response;
		std::string					_buffer;
		time_t 						_requestStartTime;

};

#endif