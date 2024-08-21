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
        void			setServerFd(int fd);
        int    			getServerFd();
		void			setConnectedServers(int serverfd, std::map<int, std::vector<VirtualServer*> >	&socketBoundVs);
		ClientStatus	&getClientStatus();
		// VirtualServer	&getConnectedServer();
		// void			setMaxBodySize(size_t maxBodySize);
		// size_t			getMaxBodySize();

        int 			readRequest(int isInReadSet);
        ResponseOutcome writeResponse();
        
    private :
        int 						_clientfd;
		int							_serverfd;
		std::vector<VirtualServer*>	_vsCandidates;
		Request						*_request;
		Response					*_response;
		std::string					_buffer;
		time_t 						_requestStartTime;
		ClientStatus				_clientStatus;
		bool						_keepAlive;

};

#endif