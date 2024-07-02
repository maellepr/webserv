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
		void			setConnectedServer(VirtualServer &vs);
		VirtualServer	&getConnectedServer();
		// void			setMaxBodySize(size_t maxBodySize);
		// size_t			getMaxBodySize();

        int readRequest();
        int writeResponse();
        
    private :
        int 			_socketfd;
		VirtualServer	_connectedVS;
		Request			*_request;
		Response		*_response;
		std::string		_buffer;
		// size_t 			_maxBodySize;

};

#endif