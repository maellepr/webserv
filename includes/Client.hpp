#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "webserv.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Response;

class Client 
{
    public :
        Client();
        ~Client();

        void			setFd(int fd);
        int    			getFd();
        void			setServerFd(int fd);
        int    			getServerFd();

		void			setClient(std::map<int, Client> &c);
		void			setSocketBoundVs(std::map<int, std::vector<VirtualServer*> > &vs);

		void			setConnectedServers(int serverfd, std::map<int, std::vector<VirtualServer*> >	&socketBoundVs);
		ClientStatus	&getClientStatus();

		void			setFdInfos(int fdMax, fd_set write_fds, fd_set read_fds);

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

		fd_set						_read_fds;
		fd_set						_write_fds;
		int							_fd_max;

		// std::vector<int>									_clients;
		std::map<int, Client>								*_c;
		std::map<int, std::vector<VirtualServer*> >			_socketBoundVs;

};

#endif