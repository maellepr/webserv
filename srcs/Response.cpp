#include "../includes/Response.hpp"

Response::Response() : _statusCode(STATUS_OK), _errorCloseSocket(false)
{
}

Response::~Response()
{
}

void	Response::setFdInfos(int fdMax, fd_set writeFds, fd_set readFds)
{
	_fd_max = fdMax;
	_write_fds = writeFds;
	_read_fds = readFds;
}

// void	Response::setClient(std::vector<int> &c)
// {
// 	_clients = c;
// }

void	Response::setClient(std::map<int, Client> *c)
{
	_c = c;
}


void	Response::setSocketBoundVs(std::map<int, std::vector<VirtualServer*> > &vs)
{
	_socketBoundVs = vs;
}

void	Response::generateResponse(ParseRequestResult &request)
{
	// std::cout << "request.hostName = " << request.hostName << std::endl;
	// MODIFIER LES ELSE IF
	std::cerr << "generate Response 1\n";
	if (request.outcome == REQUEST_FAILURE) //parsing failure
	{
		std::cerr << "generate Response 2\n";
		buildErrorPage(request, request.statusCode);
		std::cerr << "generate Response 3\n";
	}
	else if (methodIsAuthorize(request) == false) // location method not allowed
	{
		buildErrorPage(request, STATUS_METHOD_NOT_ALLOWED);
	}
	else if (request.location->getReturn().size() > 0) // redirection return + gere les infinite loop !
	{
		std::cerr << TURQUOISE << ">> Return <<\n" << RESET;
		buildReturn(request);
		std::cerr << TURQUOISE << ">> end of return <<\n" << RESET;
	}
	else
	{
		if (request.location->getConfigLocation().find("cgi") != request.location->getConfigLocation().end()) // CGI
		{
			std::cerr << PINK << ">> CGI <<\n" << RESET;
			buildCgi(request);
			std::cerr << PINK << ">> end of CGI <<\n" << RESET;
		}
		else if (request.method == GET)
		{
			std::cerr << "generate Response 4\n";
			buildGet(request);
			std::cerr << "generate Response 5\n";
		}
		else if (request.method == POST)
		{
			std::cerr << "generate Response 6\n";
			buildPost(request);
			std::cerr << "generate Response 7\n";
		}
		else if (request.method == DELETE)
		{
			std::cerr << PURPLE << ">> Delete <<\n" << RESET;
			buildDelete(request);
			std::cerr << PURPLE << ">> end of delete <<\n" << RESET;
		}
	}

	buildStatusLine();
	// std::cerr << "_body = " << _body << std::endl;
	std::cerr << "generate Response 6\n";
	// build headers (+body)
}

bool	Response::loopDetectedReturn(ParseRequestResult &request)
{
	std::string returnUri;
	for (std::map<int, std::string>::iterator i = request.location->getReturn().begin(); i != request.location->getReturn().end(); i++)
	{
		returnUri = i->second;
	}
	if (returnUri.size() > 0 && returnUri[0] != '/')
	{
		returnUri.insert(0, "/");
	}
	std::cerr << "returnUri = " << returnUri << "\n";
	// On cherche a comparer la prochaine location a laquelle correspond returnUri
	// avec la location actuelle, si c'est la meme -> infinite loop
	// exact match
	// std::cerr << "exact match\n";
	for (std::map<std::string, Location>::iterator it = request.vs->getLocationsEqual().begin(); it != request.vs->getLocationsEqual().end(); it++)
	{// on cherche dans toutes les locations du virtual server avec un prefix "="
	 // si un prefix vs correspond a returnUri et qu'il est egal a la loc actuelle de notre request -> loop Detected
		// std::cerr << "it->first : " << it->first << "\n";
		if (it->first == returnUri && it->first == request.location->getPrefix())
		{
			// std::cerr << "IT->FIRST : " << it->first << "\n";
			return true;
		}
	}
	// longuest match
	// std::cerr << "longuest match\n";
	size_t 		len(0);
	std::string	locationPrefix;
	// Location 	*locationTmp;
	for (std::map<std::string, Location>::iterator it = request.vs->getLocations().begin(); it != request.vs->getLocations().end(); it++)
	{// on cherche dans toutes les locations du virtual server
	 // si un prefix correspond au debut de l'uri 
	 // et si l'uri est plus grande que la length precedente 
	 // sauver la location, et changer len, si jamais il y a un meilleur match
		// std::cerr << "it->first : " << it->first << "\n";
		if (it->first == returnUri.substr(0, it->first.size()))
		{
			if (it->first.size() > len)
			{
				// std::cerr << "IT->FIRST : " << it->first << "\n";
				locationPrefix = it->first;
				// locationTmp = &(it->second);
				len = it->first.size();
			}
		}
	}
	// std::cerr << "locationPrefix : " << locationPrefix << "\n";
	if (locationPrefix == request.location->getPrefix())
		return true ;
	return false ;
}

void	Response::buildReturn(ParseRequestResult &request)
{
	if (loopDetectedReturn(request) == true)
	{
		// std::cerr << "loop detected in the return section\n";
		_statusCode = STATUS_LOOP_DETECTED;
		return(buildErrorPage(request, _statusCode));
	}
	int			returnCode;
	std::string	redirectUri;
	// std::cerr << "get Return :\n";
	for (std::map<int, std::string>::iterator i = request.location->getReturn().begin(); i != request.location->getReturn().end(); i++)
	{
		// std::cerr << i->first << " " << i->second << "\n";
		returnCode = i->first;
		redirectUri = i->second;
	}
	if (returnCode == 300)
		_statusCode = STATUS_MULTIPLE_CHOICES;
	else if (returnCode == 301)
		_statusCode = STATUS_MOVED_PERMANENTLY;
	else if (returnCode == 302)
		_statusCode = STATUS_FOUND;
	else if (returnCode == 303)
		_statusCode = STATUS_SEE_OTHER;
	else if (returnCode == 304)
		_statusCode = STATUS_NOT_MODIFIED;
	else if (returnCode == 305)
		_statusCode = STATUS_USE_PROXY;
	else if (returnCode == 306)
		_statusCode = STATUS_SWITCH_PROXY;
	else if (returnCode == 307)
		_statusCode = STATUS_TEMPORARY_REDIRECT;
	else if (returnCode == 308)
		_statusCode = STATUS_PERMANENT_REDIRECT;

	// _headers["HTTP/1.1 301 Moved Permanently\r\n"] = "";
	_headers["location"] = "/" + redirectUri + "\r\n";
	// _headers["content-length"] = "0\r\n";
}

bool	Response::methodIsAuthorize(ParseRequestResult &request)
{
	// std::cerr << "request config location :\n";
	// for (std::map<std::string, std::vector<std::string> >::iterator i = request.location->getConfigLocation().begin(); i != request.location->getConfigLocation().end(); i++)
	// {
	// 	std::cerr << i->first << "\n";
	// 	for (std::vector<std::string>::iterator j = i->second.begin(); j != i->second.end(); j++)
	// 	{
	// 		std::cerr << *j << " ";
	// 	}
	// 	std::cerr << "\n";
	// }


	std::map<std::string, std::vector<std::string> >::iterator it = request.location->getConfigLocation().find("methods");
	std::string m;
	if (request.method == POST)
		m = "POST";
	else if (request.method == GET)
		m = "GET";
	else if (request.method == DELETE)
		m = "DELETE";
	else
		m = "NONE";// peut etre ajouter erreur
	std::cerr << PINK << BOLD << "request method : " << m << "\n" << RESET;
	if (it != request.location->getConfigLocation().end())
	{
		for (std::vector<std::string>::iterator itSec = it->second.begin(); itSec != it->second.end(); itSec++)
		{
			
			if (request.method == POST && *itSec == "POST")
			{
				std::cerr << PINK << BOLD << "POST allowed\n" << RESET;
				return true ;
			}
			else if (request.method == GET && *itSec == "GET")
			{
				std::cerr << PINK << BOLD << "GET allowed\n" << RESET;
				return true ;
			}
			else if (request.method == DELETE && *itSec == "DELETE")
			{
				std::cerr << PINK << BOLD << "DELETE allowed\n" << RESET;	
				return true ;
			}
		}

		std::cerr << PINK << BOLD << "method " << m << " not allowed\n" << RESET;
		return false;
	}
	std::cerr << PINK << BOLD << "all methods allowed\n" << RESET;
	return true;
}

void	Response::buildDelete(ParseRequestResult &request)
{
	
	if (request.uri.size() > 0 && request.uri[0] == '/')
		request.uri = request.uri.substr(1, request.uri.size() - 1);
	std::cerr << "request uri in delete : " << request.uri << "\n";	
	if (remove(request.uri.c_str()) != 0)
	{
		std::cerr << "remove succed\n";
		_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><link href=\"./style/style.css\" rel=\"stylesheet\"><link href=\"./style/error_page.css\" rel=\"stylesheet\"><title>Ressource deleted</title></head><body><h1>Ressource deleted</h1><p id=\"comment\">The ressource was successfully deleted.</p><p><a href=\"site_index.html\"><button>Index</button></a></p></body></html>";
	}
	else
	{
		std::cerr << "remove failed\n";
		_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><link href=\"./style/style.css\" rel=\"stylesheet\"><link href=\"./style/error_page.css\" rel=\"stylesheet\"><title>Ressource not deleted</title></head><body><h1>Ressource not deleted</h1><p id=\"comment\">The ressource you are trying to delete does not exist.</p><p><a href=\"site_index.html\"><button>Index</button></a></p></body></html>";
	}
}

void	Response::buildCgi(ParseRequestResult &request)
{
	initCgi(request);
	if (_statusCode != STATUS_OK)
		return(buildErrorPage(request, _statusCode));
	// std::cerr << "max fd = " << _fd_max << "\n";
	// ***************************************************************************
	std::cout << "request.body : " << request.body << std::endl;
	int fdBody = open("/tmp/.sendBody.txt", O_RDWR | O_CREAT);
	// std::ofstream fillBody;
	// fillBody.open("/tmp/.sendBody.txt");
	// fillBody << request.body;
	// fillBody.close();
	// ***************************************************************************
	int	fd[2]; // ***************************************************************************
	fd[0] = fdBody; // ***************************************************************************
	int writeStatus;
	std::string cgiFile	= ".read_cgi.txt";
	int	cgiFd = open(cgiFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
	if (cgiFd != -1)
	{
		if (chmod(cgiFile.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		{
			close(cgiFd);
			return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));
		}
	}
	if (pipe(fd) == - 1)// ***************************************************************************
		return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));// ***************************************************************************

	std::string cgi = findCgi();
		_cgi = const_cast<char*>(cgi.c_str());
	if (cgi.size() == 0)
		return (buildErrorPage(request, STATUS_BAD_REQUEST));

	std::vector<std::string> vecEnv = doEnvCgi(request);
	time_t	start = time(NULL);
	pid_t	pid = fork();
	if (pid == -1)
		return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));
	if (pid == 0)//child
	{
		// std::cerr << "CHILD 0" << std::endl;
		dup2(fd[0], STDIN_FILENO);//stdin devient pipe[0] // ***************************************************************************
		close(fd[0]);// ***************************************************************************
		// close(fd[1]);
		dup2(cgiFd, STDOUT_FILENO);//stdout devient pipe[1]
		close(cgiFd);
		// size_t extention = _finalURI.find_last_of('.');
		// std::cerr << "_finalURI = " << _finalURI << "\n";
		// dprintf(2, "size = %lu", extention);
		// std::string ext = _finalURI.substr(extention, _finalURI.size());
		// std::cerr << "extention = " << ext << "\n";

	std::ofstream fillBody; // ***************************************************************************
	fillBody.open("/tmp/.sendBody.txt"); // ***************************************************************************
	fillBody << request.body; // ***************************************************************************
	fillBody.close(); // ***************************************************************************

		dprintf(2, "_cgi here = %s\n", _cgi);
		char *av[] = {_cgi, _finalUriChar, NULL};

		char **env = vectorStringToChar(vecEnv);
		closeAllFd();
		// std::cerr << "CHILD 1" << std::endl;
		execve(_cgi, av, env);
		// std::cerr << "CHILD 2" << std::endl;
		// execve failed
		perror("execve");
		freeChar(env);
		sleep(3);
		exit(EXIT_FAILURE);
	}
	// system("pstree -p");
	// close(fd[1]);
	// close(fd[0]);
	while (true)
	{
		// std::cerr << "PARENT 0" << std::endl;
		pid_t	pid_result = waitpid(pid, &writeStatus, WNOHANG);
		if (pid_result > 0)// success, child process change state
		{
			std::cerr << "waitpid success, child process chage state\n";
			break;
		}
		if (pid_result == 0)// no state change detected, verify that not too much time passed (infinite loop ?)
		{
			time_t	end = time(NULL);
			if (end - start >= 10)// valeur 3?
			{
				std::cerr << BOLD << "return error page timeout\n" << "pid killed = " << pid << "\n" << RESET;
				close(cgiFd);
				if (kill(pid, SIGKILL) == 0)
					std::cerr << "Child process killed successfully\n";
				else
					std::cerr << "Failed to kill child process\n";
				pid_result = waitpid(pid, &writeStatus, WNOHANG);
				if (kill(pid, SIGKILL) == 0)
					std::cerr << "Child process killed successfully\n";
				_statusCode = STATUS_INTERNAL_SERVER_ERROR;
				remove(".read_cgi.txt");
				remove("/tmp/.sendBody.txt"); // *****************************************************************
				return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));
			}
		}
		else// waitpid failed
		{
			perror("waitpid");
			close(cgiFd);
			return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));
		}
	}
	close(cgiFd);
	buildPageCgi();	
	if (_statusCode != STATUS_OK)
		buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR);
}

std::string	Response::findCgi()
{
	size_t extention = _finalURI.find_last_of('.');
	// std::cerr << "_finalURI = " << _finalURI << "\n";
	// dprintf(2, "size = %lu", extention);
	std::string ext = _finalURI.substr(extention, _finalURI.size());
	// std::cerr << "extention = " << ext << "\n";
	std::string cgi;

	if (ext == ".py")
		cgi = "/usr/bin/python3";
	else if (ext == ".php")
		cgi = "/usr/bin/php";
	else
		cgi = "";
	return cgi;
	// _cgi = const_cast<char*>(cgi.c_str());
	// dprintf(2, "_cgi = %s\n", _cgi);
}

void	Response::closeAllFd(void)
{
	// std::cerr << "class all -> fd_max : " << _fd_max << "\n";
	// // _fd_max += 13;
	// for (int fd = 3; fd <= _fd_max; fd++)
	// {
	// 	std::cerr << PINK << "close all fd in child\n" << RESET;
	// 	std::cerr << "fd_max : " << _fd_max << "\n";
	// 	if (FD_ISSET(fd, &_write_fds))
	// 		FD_CLR(fd, &_write_fds);
	// 	if (FD_ISSET(fd, &_read_fds))
	// 		FD_CLR(fd, &_read_fds);
		
	// 	// if (fd == _fd_max)
	// 	// 	_fd_max--;
	// 	std::cerr << "fd about to be closed " << fd << "\n";
	// 	close(fd);
	// }

	for (std::map<int, std::vector<VirtualServer*> >::iterator it = _socketBoundVs.begin(); it != _socketBoundVs.end(); it++)
	{
		// for (std::vector<VirtualServer*>::iterator vsIt = it->second.begin(); vsIt != it->second.end(); vsIt++)
		// {
		// 	close((*vsIt)->getSocketFd());
		// }
		std::cerr << PINK << "close socket server :" << it->first << RESET << "\n";
		close(it->first);
	}

	// close client sockets
	std::cerr << "client size = " << _c->size() << "\n";
	for(std::map<int, Client>::iterator it = _c->begin(); it != _c->end(); it++)
	{
		std::cerr << PINK << "close socket client :" << it->second.getFd() << RESET << "\n";
		close(it->first);
	}
	
	// for (size_t i = 3; i < 15; i++)
	// {
	// 	std::cerr << PINK << "close socket client :" << i << RESET << "\n";
	// 	close(i);
	// }
}

void	Response::buildPageCgi()
{
	std::ifstream	ifs(".read_cgi.txt");
	if (!ifs.is_open())
	{
		_statusCode = STATUS_INTERNAL_SERVER_ERROR;
		return ;
	}

	std::stringstream response;
	response << ifs.rdbuf();

	remove(".read_cgi.txt");
	remove("/tmp/.sendBody.txt"); // *****************************************************************
	_body = response.str();
	if (_body.size() == 0)
	{
		std::cerr << "response body size = 0\n";
		_statusCode = STATUS_INTERNAL_SERVER_ERROR;
		return ;
	}
	std::cerr << "response : " << _body << "\n";
	_headers["content-length"] = convertToStr(_body.size());

	size_t pos = _finalURI.find_last_of("/");
	if (pos != std::string::npos && (_finalURI.begin() + pos + 1) != _finalURI.end())
	{
		std::string resourceName = _finalURI.substr(pos + 1);
		pos = resourceName.find_last_of(".");
		if (pos != std::string::npos && (resourceName.begin() + pos + 1) != resourceName.end())
		{
			std::string fileExtension = resourceName.substr(pos + 1);
			if (CONTENT_TYPES.find(fileExtension) != CONTENT_TYPES.end())		
			{
				_headers["content-type"] = CONTENT_TYPES[fileExtension];
				return ;
			}
		}
	}
}

void	Response::initCgi(ParseRequestResult &request)
{
	if (request.query.empty())
		request.query = request.body;
	_configLocation = request.location->getConfigLocation();
	std::map<std::string, std::vector<std::string> >::iterator it = _configLocation.find("cgi");
	std::string path = it->second[0];
	path.insert(0, "./");
	// char *pathCgi = const_cast<char*>(path.c_str());
	if (_configLocation.find("rootDir") != _configLocation.end())
		_rootDir = _configLocation["rootDir"][0];// rootDir : www/cgiTest/cgi-bin
	if (_rootDir[0] == '/')
		_rootDir = _rootDir.substr(1, _rootDir.size() - 1);
	if (_rootDir[_rootDir.size() -1] == '/')
		_rootDir = _rootDir.substr(0, _rootDir.size() - 1);
	_finalURI = _rootDir + request.uri;// www/cgiTest/cgi-bin/index
	_finalURI.insert(0, "./");
	_finalUriChar = const_cast<char*>(_finalURI.c_str());
	// dprintf(2, "_finalUriChar = %s\n", _finalUriChar);
	if (access(_finalUriChar, F_OK) != 0)
		_statusCode = STATUS_NOT_FOUND;
}

std::vector<std::string>	Response::doEnvCgi(ParseRequestResult &request)
{
	std::vector<std::string> env;

	std::stringstream ss;
	ss << request.contentLenght;
	// std::cerr << "request.contentLenght = " << request.contentLenght << "\n";
	// std::cerr << "content lenght = " << ss.str() << "\n";
	exportToEnv(env, "CONTENT_LENGTH", ss.str());
	std::map<std::string, std::string>::iterator it = request.headers.find("content-type");
	if (it == request.headers.end())
		exportToEnv(env, "CONTENT_TYPE", DEFAULT_CONTENT_TYPE);
	else
		exportToEnv(env, "CONTENT_TYPE", it->second);
	exportToEnv(env, "DOCUMENT_ROOT", "/www/html/");
	exportToEnv(env, "GATEWAY_INTERFACE", CGI_VERSION);
	// std::cerr << "request_headers = \n";
	for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++)
	{
		// std::cerr << it->first << " " << it->second << "\n";
		std::string keyWord = it->first;
		int i = 0;
		while (keyWord[i])
		{
			keyWord[i] = toupper(keyWord[i]);
			if (keyWord[i] == '-')
				keyWord[i] = '_';
			i++;
		}
		keyWord.insert(0, "HTTP_");
		if (keyWord != "HTTP_CONNECTION")
			exportToEnv(env, keyWord, it->second);
	}
	std::string absPath = getAbsPath(_finalURI);
	// std::cerr << "absPath == " << absPath << "\n";
	std::string pathInfo = "/mnt/nfs/homes/mapoirie/Documents/webserv_git/www" + request.uri;
	std::cerr << "pathInfo : " << pathInfo << "\n";
	exportToEnv(env, "PATH_INFO", pathInfo);

	exportToEnv(env, "QUERY_STRING", request.query);
	exportToEnv(env, "REDIRECT_STATUS", "200");// 200 to indicate the requesst was hande correctly
	std::string	method;
	if (request.method == GET)
		method = "GET";
	else if (request.method == POST)
		method = "POST";
	else if (request.method == DELETE)
		method = "DELETE";
	
	exportToEnv(env, "REQUEST_METHOD", method);
	// std::cerr << "request.uri =>> " << request.uri << "\n";
	// exportToEnv(env, "SCRIPT_NAME", request.uri);
	exportToEnv(env, "SCRIPT_NAME", request.uri);// A CHANGER en fonction
	exportToEnv(env, "SCRIPT_FILENAME", pathInfo);
	exportToEnv(env, "SERVER_PROTOCOL", PROTOCOL_VERSION);
	exportToEnv(env, "SERVER_SOFTWARE", SERVER_SOFTWARE);
	// std::cerr << "Response headers = \n";
	// for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++)
	// {
	// 	std::cerr << it->first << "\n";
	// 	std::cerr << it->second << "\n"; 
	// }
	// std::cerr << "ENV =\n";
	// for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); it++)
	// {
	// 	std::cerr << (*it) << "\n";
	// }
	return (env);
}

void	Response::exportToEnv(std::vector<std::string> &env, const std::string &key, const std::string &value)
{
	env.push_back(key + "=" + value);
}

void	Response::buildStatusLine()
{
	// std::stringstream ss;
	// ss << _statusCode;
	// _statusLine = std::string(PROTOCOL_VERSION) + " " + ss.str() + " " + STATUS_MESSAGES[_statusCode] + "\r\n";
	_statusLine = std::string(PROTOCOL_VERSION) + " " + convertToStr(_statusCode) + " " + STATUS_MESSAGES[_statusCode] + "\r\n";
	// std::cout << GREEN << "statusLine = " << _statusLine << RESET << std::endl;
}

void	Response::buildErrorPage(ParseRequestResult &request, StatusCode statusCode)
{
	if (statusCode != STATUS_NOT_FOUND)
		_errorCloseSocket = true;
	// Attention, le request.statusCode n'est plus forcement valide => utilise celui envoye dans les arguments
	std::cerr << "build error page\n";
	std::string	errorPageUri("");
	_statusCode = statusCode;
	if (request.location)
	{
		std::cout << "STATUSCODE = " << _statusCode << std::endl;
		std::map<int, std::string>::iterator loc = request.location->getErrorPages().find(_statusCode);
		if (loc != request.location->getErrorPages().end())
		{
			std::cout << "Found StatusCode\n";
			errorPageUri = loc->second;
			// errorPageUri = "." + _rootDir + loc->second;
			std::cerr << "errorPageUri = " << errorPageUri << "\n";
		}
	}
	if (errorPageUri.empty() || !readContent(errorPageUri, _body))
	{
		std::cerr << "PAS DE PAGE ERROR RECORDED\n";
		std::string errorMsg;
		std::map<StatusCode, std::string>::iterator it = STATUS_MESSAGES.find(_statusCode);
		if (it != STATUS_MESSAGES.end())
			errorMsg = convertToStr(_statusCode) + " " + STATUS_MESSAGES[_statusCode];
		else
			errorMsg = "Unknown error " + convertToStr(_statusCode);
		// std::stringstream ss;
		// ss << _statusCode;
		// std::string	codeStr = ss.str(); 
		// std::string	title;
		// if (it != STATUS_MESSAGES.end())
		// 	"Unknown error " + codeStr;
		// else
		// 	codeStr + " " + it->second;
		// _body = "<!DOCTYPE html>\n"
		// 			"<html lang=\"en\">\n"
		// 			"\n"
		// 			"<head>\n"
		// 			"\t<meta charset=\"UTF-8\">\n"
		// 			"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		// 			"\t<title>" +
		// 			title +
		// 			"</title>\n"
		// 			"\t<style>\n"
		// 			"\t\tbody {\n"
		// 			"\t\t\tbackground-color: #f0f0f0;\n"
		// 			"\t\t\tfont-family: Arial, sans-serif;\n"
		// 			"\t\t}\n"
		// 			"\n"
		// 			"\t\t.container {\n"
		// 			"\t\t\twidth: 80%;\n"
		// 			"\t\t\tmargin: auto;\n"
		// 			"\t\t\ttext-align: center;\n"
		// 			"\t\t\tpadding-top: 20%;\n"
		// 			"\t\t}\n"
		// 			"\n"
		// 			"\t\th1 {\n"
		// 			"\t\t\tcolor: #333;\n"
		// 			"\t\t}\n"
		// 			"\n"
		// 			"\t\tp {\n"
		// 			"\t\t\tcolor: #666;\n"
		// 			"\t\t}\n"
		// 			"\t</style>\n"
		// 			"</head>\n"
		// 			"\n"
		// 			"<body>\n"
		// 			"\t<div class=\"container\">\n"
		// 			"\t\t<h1>" +
		// 			title +
		// 			"</h1>\n"
		// 			"\t\t<a href=\"/\">Go back to root.</a>\n"
		// 			"\t</div>\n"
		// 			"</body>\n"
		// 			"\n"
		// 			"</html>";

	_body += "<!DOCTYPE html>\n";
	_body += "<html>\n";
	_body += "<body>\n";
	_body += "<h1>" + errorMsg + "</h1>";
	_body += "</body>\n";
	_body += "</html>";
	
	}
	_headers["content-type"] = "text/html";
	_headers["content-length"] = convertToStr(_body.size());

	// std::cerr << "BODY =" << _body << std::endl;
		// error_page in location 
		// or
		// build error page from scratch
}

void	Response::buildGet(ParseRequestResult &request)
{
	_configLocation = request.location->getConfigLocation();
	if (_configLocation.find("rootDir") != _configLocation.end())
		_rootDir = _configLocation["rootDir"][0];
	std::cout << "root = " << _rootDir << std::endl;
	if (_rootDir[0] == '/')
		_rootDir = _rootDir.substr(1, _rootDir.size() - 1);
	if (_rootDir[_rootDir.size() -1] == '/')
		_rootDir = _rootDir.substr(0, _rootDir.size() - 1);
	_finalURI = _rootDir + request.uri;
	std::cout << "_finalURI = " << _finalURI << std::endl;

	// if (isUriValid(_finalURI) == false)
	// {
	// 	std::cout << "URI INVALID" << std::endl;
	// 	return (buildErrorPage(request, STATUS_FORBIDDEN));
	// }
	if (isPathADirectory(_finalURI))
	{
		std::cerr << "Path is a directory\n";
		if (_finalURI[_finalURI.size() -1] != '/')
		{
			// request.statusCode = STATUS_MOVED_PERMANENTLY;
			_statusCode = STATUS_MOVED_PERMANENTLY;
			std::string serverName(request.hostName);
			if (serverName.empty())
				serverName = request.vs->getIP() + ":" + convertToStr(request.vs->getPort());
			else
				serverName += ":" + convertToStr(request.vs->getPort());
			_headers["location"] = "http://" + serverName + request.uri + "/" + "\r\n"; //A mettre ici ou dans builHeaders ?
			return ;
		}
		else
		{
			std::cerr << "CASE 0\n";
			if (_configLocation.find("index") != _configLocation.end())
			{
				std::cerr << "CASE 1\n";
				std::vector<std::string> indexPages = _configLocation["index"];
				Location *newLocation = NULL;
				if (indexPages.empty() == false)
				{
					std::cerr << "CASE 1.1\n";
					for (std::vector<std::string>::iterator it = indexPages.begin(); it != indexPages.end(); it++)
					{
						std::string index = (*it)[0] == '/' ? (*it).substr(1, std::string::npos) : (*it);
						std::string path;
						path = _finalURI + index;
						std::cout << "path = " << path << std::endl;
						if (isPathADRegularFile(path))
						{
							std::cout << "path regular"<< std::endl;
							_finalURI = path;
							return (buildPage(request));
						}
						std::cerr << "CASE 1.2\n";
						if (request.location->getEqualModifier() == true && newLocation == NULL)
						{
							newLocation = associateLocationResponse(request, index);
						}
					}
				}
				if (newLocation)
				{
					std::cerr << "CASE 1.3\n";
					request.location = newLocation;
					return (generateResponse(request));
				}
				std::cerr << "CASE 1.4\n";
			}
			if (_configLocation.find("autoindex") != _configLocation.end()
					&& _configLocation["autoindex"][0] == "true")
			{
					std::cerr << "CASE 1.5\n";
					return(buildAutoindexPage(request));
			}
			std::cerr << "CASE 1.6\n";
			return (buildErrorPage(request, STATUS_FORBIDDEN));
		}
	}
	std::cerr << "before build page\n";
	if (isPathADRegularFile(_finalURI))
	{
		std::cerr << "build page\n";
		return (buildPage(request));
	}
	else
	{
		std::cerr << "PAS build page\n";
		return (buildErrorPage(request, STATUS_NOT_FOUND));
	}
}

void	Response::buildPost(ParseRequestResult &request)
{
	std::cout << LIGHTBLUE << "BUILDPOST\n" << RESET;
	if (request.isUpload)
	{
		listUploadFiles(request);
		if (_uploads.empty())
		{
			_statusCode = STATUS_NO_CONTENT;
			return ;
		}
		for (std::map<std::string, std::string>::iterator it = _uploads.begin(); it != _uploads.end(); it++)
		{
			std::string filename = "./www/uploads/" + it->first;
			if (access(filename.c_str(), F_OK) == 0)
			{
				// std::cout << RED << "FILE ALREADY EXISTS" << RESET << std::endl;
				std::string fileRename = filename;
				for (std::size_t i = 1; i < 11; i++) // 10 copies max
				{
					std::size_t extensionPos = filename.find_last_of(".");
					if (extensionPos != std::string::npos && extensionPos != 0)
						fileRename = filename.substr(0, extensionPos);
					fileRename += "(" + convertToStr(i) + ")";
					if (extensionPos != std::string::npos && extensionPos != 0)
						fileRename += filename.substr(extensionPos, std::string::npos);
					if (access(fileRename.c_str(), F_OK) != 0)
					{
						filename = fileRename;
						break ;
					}
				}
			}
			std::cerr << "before access = " << filename.c_str() << "\n";
			if (access(filename.c_str(), F_OK) != 0)
			{
				std::ofstream fileToUpload;
				fileToUpload.open(filename.c_str(), std::ofstream::binary);
				std::vector<unsigned char> v = vectorizeString(it->second);
				for (std::vector<unsigned char>::iterator vit = v.begin(); vit != v.end(); vit++)
				{
					fileToUpload << *vit;
				}
				fileToUpload.close();
				_statusCode = STATUS_CREATED;
				_finalURI = "./www/upload_success.html";
				return (buildPage(request));
			}
			else
			{
				return (buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR));
			}
		}
	}
	else
		return (buildGet(request));
	return ;
}

void	Response::listUploadFiles(ParseRequestResult &request)
{
	std::size_t boundaryPos = request.body.find(request.boundary + "\r\n", 0);
	if (boundaryPos != 0)
		return(buildErrorPage(request, STATUS_BAD_REQUEST));
	// std::cout << LIGHTBLUE << "BUILDPOST 1\n" << RESET;
	std::size_t beginPos = boundaryPos + request.boundary.size() + strlen("\r\n");
	// std::cout << DARKYELLOW << "REQBODY :\n" << request.body << RESET << std::endl;
	while (boundaryPos != std::string::npos)
	{
		// std::cout << LIGHTBLUE << "BUILDPOST 2\n" << RESET;
		// find next upload
		boundaryPos = request.body.find(request.boundary, beginPos);
		if (boundaryPos == std::string::npos)
			break ;
		// std::cout << DARKYELLOW << "beginPos = " << beginPos << RESET << std::endl;
		// std::cout << DARKYELLOW << "boundaryPos = " << boundaryPos << RESET << std::endl;
		std::string subBody = request.body.substr(beginPos, boundaryPos - beginPos);
		beginPos = boundaryPos + request.boundary.size();
		// std::cout << DARKYELLOW << "SUBBODY :\n" << subBody << RESET << std::endl;
		// std::cout << LIGHTBLUE << "BUILDPOST 3\n" << RESET;

		// find filename
		std::size_t filenameStart = subBody.find("filename=", 0);
		if (filenameStart == std::string::npos)
			return(buildErrorPage(request, STATUS_BAD_REQUEST));
		// std::cout << LIGHTBLUE << "BUILDPOST 4\n" << RESET;
		filenameStart += strlen("filename=") + 1;
		std::size_t filenameEnd = subBody.find("\"", filenameStart);
		if (filenameEnd == std::string::npos)
			return(buildErrorPage(request, STATUS_BAD_REQUEST));
		// std::cout << LIGHTBLUE << "BUILDPOST 5\n" << RESET;
		std::string filename = subBody.substr(filenameStart, filenameEnd - filenameStart);

		// find upload data
		std::size_t dataStart = subBody.find("\r\n\r\n", 0);
		if (dataStart == std::string::npos)
			return(buildErrorPage(request, STATUS_BAD_REQUEST));
		// std::cout << LIGHTBLUE << "BUILDPOST 6\n" << RESET;
		dataStart += strlen("\r\n\r\n");
		std::size_t dataEnd = subBody.find("\r\n", dataStart);
		if (dataEnd == std::string::npos)
			return(buildErrorPage(request, STATUS_BAD_REQUEST));
		// std::cout << LIGHTBLUE << "BUILDPOST 7\n" << RESET;
		std::string uploadData = subBody.substr(dataStart, dataEnd - dataStart);

		// add new file contents (name + data) to the map
		if (uploadData.empty() == false)
			_uploads[filename] = uploadData;
		// std::cout << LIGHTBLUE << "BUILDPOST 8\n" << RESET;
	}
	// for (std::map<std::string, std::string>::iterator it = _uploads.begin(); it != _uploads.end(); it++)
	// {
	// 	std::cout << DARKYELLOW << "[ " << it->first << " ] : \n" << it->second << RESET << std::endl;
	// }
	return ;
}

void	Response::buildPage(ParseRequestResult &request)
{
	std::cerr << "dans build page\n";
	std::ifstream fileRequested(_finalURI.c_str());
	if (fileRequested.good() == false)
	{
		std::cerr << "file not good\n";
		return(buildErrorPage(request, STATUS_NOT_FOUND));
	}
	std::stringstream buffer;
	buffer << fileRequested.rdbuf();
	_body = buffer.str();
	if (_body.size() > request.vs->getMaxBodySize())
	{
		_body = "";
		return (buildErrorPage(request, STATUS_FORBIDDEN));
	}
	_headers["content-length"] = convertToStr(_body.size());

	size_t pos = _finalURI.find_last_of("/");
	if (pos != std::string::npos && (_finalURI.begin() + pos + 1) != _finalURI.end())
	{
		std::string resourceName = _finalURI.substr(pos + 1);
		pos = resourceName.find_last_of(".");
		if (pos != std::string::npos && (resourceName.begin() + pos + 1) != resourceName.end())
		{
			std::string fileExtension = resourceName.substr(pos + 1);
			if (CONTENT_TYPES.find(fileExtension) != CONTENT_TYPES.end())		
			{
				_headers["content-type"] = CONTENT_TYPES[fileExtension];
				// _headers["content-disposition"] = "attachment";
				return ;
			}
		}
	}
}

std::vector<std::string> Response::doDirListing(DIR *dir)
{
	std::vector<std::string> filesList;
	struct dirent *fileRead;
	while ((fileRead = readdir(dir)) != NULL)
	{
		if (strcmp(fileRead->d_name, ".") != 0 || (strcmp(fileRead->d_name, "..") != 0 && _finalURI != "/"))
			filesList.push_back(fileRead->d_name);
		//directory : a-t-on bien le slash de fin?
	}
	return (filesList);
}

void	Response::buildAutoindexPage(ParseRequestResult &request)
{
	std::vector<std::string> filesList;
	DIR *dir = opendir(_finalURI.c_str());
	if (!dir)
		buildErrorPage(request, STATUS_INTERNAL_SERVER_ERROR);
	filesList = doDirListing(dir);

	_headers["content-type"] = "text/html";
	_body += "<!DOCTYPE html>\n";
	_body += "<html>\n";
	_body += "<body>\n";
	_body += "<h1>Index of " + _finalURI + "</h1>";

	for (std::vector<std::string>::iterator it = filesList.begin(); it != filesList.end(); it++)
	{
		if (*it == ".")
			continue ;
		std::string hyperlink("");
		std::string filename("");
		if (*it == "..")
			filename = "<< go back";
		else
			filename = (*it);
		// hyperlink = _finalURI + (*it);
		hyperlink = (*it);
		std::cout << "hyperlink = " << hyperlink << std::endl;
		_body += "<p><a href=" + hyperlink + ">" + filename + "</a></p>\n";
	}

	_body += "</body>\n";
	_body += "</html>";

	closedir(dir);

	_headers["content-length"] = convertToStr(_body.size());
}

ResponseOutcome	Response::sendResponseToClient(int fd)
{
	std::string	line;
	
	// if (_returnRes.size() > 0)
	// {
	// 	std::cerr << "-------->_returnRes to push = " << _returnRes << "\n";
	// 	if (pushStrToClient(fd, _returnRes) == -1)
	// 		return RESPONSE_FAILURE;
	// 	return RESPONSE_SUCCESS;
	// }

	std::cerr << "-------->_statusLine to push = " << _statusLine << "\n";
	if (pushStrToClient(fd, _statusLine) == -1)
		return RESPONSE_FAILURE;

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	{
		line = it->first + ": " + it->second + "\r\n";
		std::cerr << "-------->line to push = " << line << "\n";
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
	}
	line = "\r\n";
	if (pushStrToClient(fd, line) == -1)
		return RESPONSE_FAILURE;

	std::map<std::string, std::string>::iterator it = _headers.find("content-length");
	// if (it != _headers.end())
		// std::cerr << "FOUND CONTENT LENGTH of " << strtol(it->second.c_str(), NULL, 10) << "\n";
	if (it != _headers.end() && strtol(it->second.c_str(), NULL, 10) > 0)
	{
		std::cerr << "OK THERE IS A BODY\n";
	 	line = _body.substr(0, strtol(it->second.c_str(), NULL, 10)); //convertir
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
		return RESPONSE_SUCCESS_KEEPALIVE;
	}
	else 
		return (RESPONSE_SUCCESS_KEEPALIVE);
	return RESPONSE_PENDING;
}
//
int	Response::pushStrToClient(int fd, std::string &str)
{
	size_t	bytesSent = 0, tmpSent = 0;
	std::cerr << "str = <" << str << ">\n";
	std::cerr << "str size = " << str.size() << "\n";
	while (bytesSent < str.size())
	{
		std::cerr << "pushstrclient 1\n";
		tmpSent = send(fd, str.c_str() + bytesSent, str.size() - bytesSent, 0);
		std::cerr << "pushstrclient 2\n";
		if (tmpSent <= 0)
			return (-1);
		bytesSent += tmpSent;
	}
	return (0);
}

Location	*Response::associateLocationResponse(ParseRequestResult &request, std::string index)
{
	size_t len(0);
	std::string	newURI = request.uri + index;
	Location *newLoc = NULL;

	// exact match
	for (std::map<std::string, Location>::iterator it = request.vs->getLocationsEqual().begin(); it != request.vs->getLocationsEqual().end(); it++)
	{
		// if (it->second.getEqualModifier() == true)
		// {
		if (it->first == newURI)
		{
			request.uri = newURI;
			return (&(it->second));
		}	
		// }
	}

	// longuest prefix
	for (std::map<std::string, Location>::iterator it = request.vs->getLocations().begin(); it != request.vs->getLocations().end(); it++)
	{
		// if (it->second.getEqualModifier() == false)
		// {
		if (newURI.substr(0, it->first.size()) == it->first)
		{
			if (it->first.size() > len)
			{
				request.uri = newURI;
				newLoc = &(it->second);
				len = it->first.size();
			}
		}
		// }
	}

	// no location available => server acts as location
	if (newLoc == NULL)
	{
		Location	location(request.vs->getReturnPages(), *request.vs, true);
		location.setPrefix("/");
		request.vs->getLocations()["/"] = location;
		newLoc = &request.vs->getLocations()["/"];
	}

	return (newLoc);
}

bool	Response::getErrorCloseSocket()
{
	return _errorCloseSocket;
}