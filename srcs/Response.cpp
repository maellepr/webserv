#include "../includes/Response.hpp"

Response::Response() : _statusCode(STATUS_OK)
{
}

Response::~Response()
{
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
	else if (request.location->getConfigLocation().find("cgi") != request.location->getConfigLocation().end()) // CGI
	{
		std::cerr << ">> CGI <<\n";
		buildCgi(request);
		(void) request;
	}
	else if (0) // location method not allowed
	{
		(void) request;
	}
	else if (0) // redirection return + gere les infinite loop !
	{
		(void) request;
	}
	else
	{
		if (request.method == GET)
		{
			std::cerr << "generate Response 4\n";
			buildGet(request);
			std::cerr << "generate Response 5\n";
		}
		if (request.method == POST)
		{
			std::cout << "GO TO BUILDPOST\n";
			return ; // buildPost(request)
		}
		if (request.method == DELETE)
			return ; // buildDelete(request)
	}

	buildStatusLine();
	// std::cerr << "_body = " << _body << std::endl;
	std::cerr << "generate Response 6\n";
	// build headers (+body)
}

void	Response::buildCgi(ParseRequestResult &request)
{
	if (request.query.empty())
		request.query = request.body;
	_configLocation = request.location->getConfigLocation();
	std::map<std::string, std::vector<std::string> >::iterator it = _configLocation.find("cgi");
	std::string path = it->second[0];
	path.insert(0, "./");
	// char	*pathCgi = const_cast<char*>(it->second[0].c_str());// dans cgiTest : usr/bin/python-3
	char *pathCgi = const_cast<char*>(path.c_str());
	std::cerr << "pathCgi = " << pathCgi << "\n";

	// (void)path;

	// std::cerr << "_rootDir = " << _rootDir << "\n";
	// std::cerr << "location uri = " << request.uri << "\n"; // = test.com
	// // std::cerr << "location->getUri = " << request.location
	// std::string uri = _rootDir + request.uri;
	
	if (_configLocation.find("rootDir") != _configLocation.end())
		_rootDir = _configLocation["rootDir"][0];// rootDir : www/cgiTest/cgi-bin
	std::cout << "root = " << _rootDir << std::endl;
	if (_rootDir[0] == '/')
		_rootDir = _rootDir.substr(1, _rootDir.size() - 1);
	if (_rootDir[_rootDir.size() -1] == '/')
		_rootDir = _rootDir.substr(0, _rootDir.size() - 1);
	std::cerr << "request-uri = " << request.uri << "\n";// page demandee : /index
	_finalURI = _rootDir + request.uri;// www/cgiTest/cgi-bin/index
	_finalURI.insert(0, "./");
	std::cout << "_finalURI = " << _finalURI << std::endl;
	std::cout << "_query = " << request.query << std::endl;
	char	*finalUri = const_cast<char*>(_finalURI.c_str());
	std::cerr << "ici\n";
	if (access(finalUri, F_OK) != 0)
		return (buildErrorPage(request, STATUS_BAD_GATEWAY));
	// A COMPLETER

	// std::cerr << "Response headers = \n";
	// for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++)
	// {
	// 	std::cerr << "KEY = " << it->first << "\n";
	// 	std::cerr << "VALUE = " << it->second << "\n"; 
	// }

	int	fd[2];
	if (pipe(fd) == - 1)
		throw ErrorResponse("Error in Response : pipe()");
	pid_t	pid = fork();
	if (pid == -1)
		throw ErrorResponse("Error in Response : fork()");
	std::cerr << "pathCgi = " << pathCgi << "\n";
	std::cerr << "finalUri = " << finalUri << "\n";
	if (pid == 0)//child
	{
		dup2(fd[0], STDIN_FILENO);// pipe[0] devient stdin
		dup2(fd[1], STDOUT_FILENO);//pipe[1] devient stdout
		close(fd[0]);
		close(fd[1]);
		char cgi[14] = "/usr/bin/php";
		char *av[] = {cgi, finalUri, NULL};
		std::vector<std::string> vecEnv = doEnvCgi(request);
		char **env = vectorStringToChar(vecEnv);
		// int i = 0;
		// std::cerr << "Env = ";
		// while (env[i])
		// {
		// 	// int j = 0;
		// 	std::cerr << env[i];
		// 	// while(env[i][j])
		// 	// {
		// 	// 	std::cerr << env[i][j] << "\n";
		// 	// 	j++;
		// 	// }
		// 	std::cerr << "\n";
		// 	i++;
		// }
		execve(cgi, av, env);
		// if execve didn't work :
		perror("execve");
		freeChar(env);
		exit(EXIT_FAILURE);
		// peut-etre faire exit
	}
	std::cerr << "PID = " << pid << " script = " << finalUri << "\n";

	// on ecrit le body dans pipe[1] (stdout)
	std::cerr << "_body_size = " << _body.size() << "\n";

	// FILE* file = fdopen(fd[1], "w");

	// std::ofstream ofs(file);
	
	// ofs << _body;
	// fclose(file);

	write(fd[1], _body.c_str(), _body.size());// A REMPLACER
	
	close(fd[1]);

	// on lit la reponse depuis pipe[0]
	std::string	response = readResponse(fd[0]);
	close(fd[0]);

	_body = response;
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


std::vector<std::string>	Response::doEnvCgi(ParseRequestResult &request)
{
	std::vector<std::string> env;

	std::stringstream ss;
	ss << request.contentLenght;
	std::cerr << "request.contentLenght = " << request.contentLenght << "\n";
	// std::cerr << "content lenght = " << ss.str() << "\n";
	exportToEnv(env, "CONTENT_LENGHT", ss.str());// a remplir avec POST
	std::map<std::string, std::string>::iterator it = request.headers.find("content-type");
	if (it == request.headers.end())
		exportToEnv(env, "CONTENT_TYPE", DEFAULT_CONTENT_TYPE);
	else
		exportToEnv(env, "CONTENT_TYPE", it->second);
	// exportToEnv(env, "DOCUMENT_ROOT", request.vs->getRoot());
	exportToEnv(env, "DOCUMENT_ROOT", "/www/html/");// A CHANGER
	//faut-il prendre root du vs ou root de la location
	//a demander a Claire : est-ce que location a forcement une rootDir? 
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
	std::cerr << "absPath == " << absPath << "\n";
	exportToEnv(env, "PATH_INFO", "/mnt/nfs/homes/mapoirie/Documents/webserv_git/www/cgi/response.php");

	exportToEnv(env, "QUERY_STRING", request.query);
	// The query string portion of the URL (the part after ? in the URL) A REMPLIR
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
	exportToEnv(env, "SCRIPT_NAME", "/cgi/response.php");
	exportToEnv(env, "SCRIPT_FILENAME", "/mnt/nfs/homes/mapoirie/Documents/webserv_git/www/cgi/response.php");
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

std::string	Response::readResponse(int fd)
{
	char buffer[16384];//BUFFER_SIZE?
	size_t length;
	std::string response;
	
	while (true)
	{
		length = read(fd, buffer, 16384 - 1);// A REMPLACE
		buffer[length] = '\0';
		response += buffer;
		if (length < 16384 - 1)
			return response ;
	}
}

void	Response::exportToEnv(std::vector<std::string> &env, const std::string &key, const std::string &value)
{
	env.push_back(key + "=" + value);
}

void			Response::buildStatusLine()
{
	// std::stringstream ss;
	// ss << _statusCode;
	// _statusLine = std::string(PROTOCOL_VERSION) + " " + ss.str() + " " + STATUS_MESSAGES[_statusCode] + "\r\n";
	_statusLine = std::string(PROTOCOL_VERSION) + " " + convertToStr(_statusCode) + " " + STATUS_MESSAGES[_statusCode] + "\r\n";
	// std::cout << GREEN << "statusLine = " << _statusLine << RESET << std::endl;
}

void			Response::buildErrorPage(ParseRequestResult &request, StatusCode statusCode)
{
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

	if (pushStrToClient(fd, _statusLine) == -1)
		return RESPONSE_FAILURE;

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	{
		line = it->first + ": " + it->second + "\r\n";
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
	}
	line = "\r\n";
	if (pushStrToClient(fd, line) == -1)
		return RESPONSE_FAILURE;
	
	// METHOD HEAD => stop and return success

	std::map<std::string, std::string>::iterator it = _headers.find("content-length");
	if (it != _headers.end())
		std::cerr << "FOUND CONTENT LENGTH of " << strtol(it->second.c_str(), NULL, 10) << "\n";
	if (it != _headers.end() && strtol(it->second.c_str(), NULL, 10) > 0)
	{
		std::cerr << "OK THERE IS A BODY\n";
	 	line = _body.substr(0, strtol(it->second.c_str(), NULL, 10)); //convertir
		if (pushStrToClient(fd, line) == -1)
			return RESPONSE_FAILURE;
		return RESPONSE_SUCCESS;
	}
	else 
		return (RESPONSE_SUCCESS);
	return RESPONSE_PENDING;
}

int	Response::pushStrToClient(int fd, std::string &str)
{
	size_t	bytesSent = 0, tmpSent = 0;
	std::cerr << "str = " << str << "\n";
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