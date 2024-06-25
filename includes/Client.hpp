#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"

class Client 
{
    public :
        Client();
        ~Client();

        void	setFd(int fd);
        int     getFd();

        int readRequest();
        int writeResponse();
        
    private :
        int _socketfd;

};

#endif