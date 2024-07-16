#include "../includes/VirtualServer.hpp"

VirtualServer::VirtualServer()
{
	memset(&_address, 0, sizeof _address);
	_address.sin_family = AF_INET; // IPv4
	_address.sin_port = htons(8080);// port par defaut
	_address.sin_addr.s_addr = htonl(INADDR_ANY);// adresse par defaut 0.0.0.0 (any address)

	_ip = "0.0.0.0";
	_port = 8080;

	_indexOnOff = false;

	_ipByDefault = true;
	_portByDefault = true;

	_maxBodySize = DEFAULT_MAXBODYSIZE;

	_toErase = false;

	_defaultVS = false;

	_isBind = 0;

	_listenState = false;
}

VirtualServer::~VirtualServer()
{
}

void	VirtualServer::init(std::istream& file)
{
	// _port = 8080;
	// _serverName = "www.minigoats.com";
	std::string	line, keyword;
	bool	empty = true;
	while (std::getline(file, line))
	{
		// std::cerr << "line vs = " << line << "\n";
		std::istringstream iss(line);

		// iss >> keyword;

		if (!(iss >> keyword) || keyword[0] == '#')
			continue ;
		// std::cerr << "keyword vs = " << keyword << "\n";
		if (keyword == "}" && empty == true)
			throw ErrorConfigFile("Error in the config file : empty server section");
		else if (keyword == "}" && empty == false)
		{
			checkNecessaryLine();
			// std::cerr << "\nSERVER :\nlisten : ip  " << _ip << " port " << _port << "\n";
			// std::cerr << "server_name : " << _serverName << "\n";
			// std::cerr << "root :" << _rootDir << "\n";
			// std::cerr << "error_pages : \n";
			// for(std::map<int, std::string>::iterator ep = _errorPages.begin(); ep != _errorPages.end(); ep++)
			// {
			// 	std::cerr << "code: " << ep->first;
			// 	std::cerr << "  page: " << ep->second << "\n";
			// }
			// std::cerr << "return :\n";
			// for(std::map<int, std::string>::iterator ret = _returnPages.begin(); ret != _returnPages.end(); ret++)
			// {
			// 	std::cerr << "code: " << ret->first;
			// 	std::cerr << "  page: " << ret->second << "\n";
			// }
			return ;
		}
		else if (keyword == "listen")
			parseListen(iss);
		else if (keyword == "server_name")
			parseServerNames(iss);
		else if (keyword == "root")
			parseRoot(iss);
		else if (keyword == "autoindex")
			parseAutoIndex(iss);
		else if (keyword == "client_max_body_size")
			parseMaxClientBodySize(iss);
		else if (keyword == "error_page")
			parseErrorPages(iss);
		else if (keyword == "index")
			parseIndex(iss);
		else if (keyword == "default_server")
			parseDefaultServer(iss);
		else if (keyword == "location")
		{
			Location	location(_returnPages, *this);
			std::string	prefix;
			if (!(iss >> keyword))
				throw ErrorConfigFile("Error in the conf file : location : wrong content1");
			if (keyword != "{")
			{
				if (keyword == "=")
				{
					location.setEqualModifier(true);
					if (!(iss >> keyword))
						throw ErrorConfigFile("Error in the conf file : location : wrong content2");
					std::cerr << "XXkeyword " << keyword << "\n";
				}
				prefix = keyword; // verifier contenu peut-etre
				location.setPrefix(prefix);
				// std::cerr << "prefix " << prefix << "\n";
				if ((iss >> keyword) && keyword != "{")
				{
					std::cerr << "keyword " << keyword << "\n";
					throw ErrorConfigFile("Error in the conf file : location : wrong content3");
				}
			}
			else
				prefix = "none";
			location.parseLocation(file);
			_location[prefix] = location;
		}
		else if (keyword == "return")
		{
			parseReturn(iss);
		}
		else
			throw ErrorConfigFile("Error in the config file : wrong keyword");
		empty = false;
	}
	return ;
}

void	VirtualServer::parseListen(std::istringstream& iss)
{
	std::string	line;
	// int	port;
	if (!(iss >> line))
		throw ErrorConfigFile("Error in the config file : listen : missing information");
	if (line.find_first_not_of("0123456789.:") != std::string::npos)
		throw ErrorConfigFile("Error in the config file : listen : wrong content");
	// std::cerr << "line pL = " << line << "\n";
	size_t	index = line.find(':');
	size_t	point = line.find('.');

	if (index != std::string::npos)// : est a ete trouve dans line
	{
		_ip = line.substr(0, index);
		_ipParse = _ip;
		parseIpAddrs();
		// std::cerr << "ipAddrs = " << ipAddrs << "\n"; 
		std::string	port = line.substr(index + 1, line.size());
		// std::cerr << "port = " << port << "\n";
		parsePort(port);
		// std::cerr << "_port = " << _port << "\n";
	}
	else if (point != std::string::npos)
	{
		_ip = line;
		_ipParse = line;
		parseIpAddrs();
	}
	else
	{
		std::string	port = line;
		// std::cerr << "port else = " << port << "\n";
		parsePort(port);
		// std::cerr << "_port else = " << _port << "\n";
	}
	if (iss >> line)
		throw ErrorConfigFile("Error in the config file : listen : wrong content");
	_listenState = true;
}

void	VirtualServer::parsePort(std::string& port)
{
	if (port.empty())
		return ;
	if (port.find_first_not_of("0123456789") != std::string::npos || port.size() > 5)
		throw ErrorConfigFile("Error in the conf file : listen : wrong port 1");
	std::stringstream ss(port);
	ss >> _port;
	if (_port > 65535)// valeur max port
		throw ErrorConfigFile("Error in the conf file : listen : wrong port 2");
	_address.sin_port = htons(_port);
	_portByDefault = false;
}

void	VirtualServer::parseIpAddrs(void)
{
	if (_ipParse.empty()/* || _ipAddrs.find_first_not_of("0123456789.") != std::string::npos*/)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)1");
	if (_ipParse == "localhost")
	{
		_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		_ipByDefault = false;
		return ;
	}
	// if (_ipParse == "*")
	// {
	// 	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	// 	_ipByDefault = false;
	// 	return ;
	// }
	if (_ipParse.find_first_not_of("0123456789.") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)2");
	// std::cerr << "ipAddrs = " << _ip << "\n";
	int	dots = 0;
	for (int i = 0; _ipParse[i]; i++)
	{	
		if (_ipParse[i] == '.')
		{
			_ipParse.replace(i, 1, 1, ' ');
			dots++;
		}
	}
	if (dots != 3)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)3");
	std::istringstream iss(_ipParse);
	std::string	more;
	int	ip[4];
	if (!(iss >> ip[0]) || !(iss >> ip[1]) || !(iss >> ip[2]) || !(iss >> ip[3]))
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)4");
	int res = 0;
	for (int i = 0; i < 4; i++)
	{
		if (ip[i] < 0 || ip[i] > 255)
			throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)5");
		res = res << 8 | ip[i];
	}
	if (iss >> more)
		throw ErrorConfigFile("Error in the conf file : listen : wrong host (ip address)6");
	// std::cerr << "res ipAddrs = " << res << "\n";
	_address.sin_addr.s_addr = htonl(res);
	_ipByDefault = false;
}

void	VirtualServer::parseServerNames(std::istringstream& iss)
{
	std::string	serverName;
	if (!(iss >> serverName))
		throw ErrorConfigFile("Error in the conf file : no server name");
	_serverName = serverName;
	if (iss >> serverName)
		throw ErrorConfigFile("Error in the conf file : server_name");
	// _serverNames.push_back(serverName);
	// while (iss >> serverName)
	// 	_serverNames.push_back(serverName);
	// _serverNameState = true;
}

void	VirtualServer::parseRoot(std::istringstream& iss)
{
	std::string	path;
	
	if (!(iss >> path))
		throw ErrorConfigFile("Error in the conf file : no root");
	// std::cerr << "path = " << path << "\n";
	if (path.compare(0, 8, "/var/www") == 0 || path.compare(0, 5, "/www/") == 0)
		_rootDir = "www";
	else
		throw ErrorConfigFile("Error in the conf file : root : wrong content1");
	// _rootDir = path + (path[path.size() - 1] == '/' ? "" : "/");
	// check if line ends with '/' -> if it does add nothing
	//							   -> if it doesn't add '/'
	if (iss >> path)
		throw ErrorConfigFile("Error in the conf file : root : wrong content2");
	if (_rootDir.empty())
		throw ErrorConfigFile("Error in the conf file : root : wrong content3");
	// std::cerr << "_rootdir = " << _rootDir << "\n"; 

    struct stat info;
    if (stat(_rootDir.c_str(), &info) != 0)// cannot access path
		throw ErrorConfigFile("Error : root : cannot access path or file");
    // if (S_ISDIR(info.st_mode) != 0)// is not a directory
	// 	throw ErrorConfigFile("Error : root : the path is not a directory");
}

void	VirtualServer::parseAutoIndex(std::istringstream& iss)
{
	if (!(iss >> _autoIndex) || _autoIndex.empty())
		throw ErrorConfigFile("Error in the conf file : auto_index not specified");
	if (_autoIndex == "on")
		_indexOnOff = true;
	else if (_autoIndex == "off")
		_indexOnOff = false;
	else
		throw ErrorConfigFile("Error in the conf file : auto_index wrong content");
}

void	VirtualServer::parseMaxClientBodySize(std::istringstream& iss)
{
	std::string	maxClientBS;

	if (!(iss >> maxClientBS) || maxClientBS.empty())
		throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong content");
	size_t	index = maxClientBS.find_first_not_of("0123456789"); 
	if (index == 0)
		throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong content");
	_maxBodySize = std::strtol(maxClientBS.c_str(), NULL, 10);
	if (maxClientBS[index] != '\0')
	{
		if (maxClientBS[index + 1] == '\0')
		{
			if (maxClientBS[index] == 'k' || maxClientBS[index] == 'K')
			{
				_maxBodySize *= KB_IN_BYTES;
			}
			else if (maxClientBS[index] == 'm' || maxClientBS[index] == 'M')
			{
				_maxBodySize *= MB_IN_BYTES;
			}
			else
				throw ErrorConfigFile("Error in the conf file : max_client_body_size : wrong unit, must be m, M, k or K");
			// std::cerr << "max client body size = " << _maxBodySize << '\n';
		}
		else
			throw ErrorConfigFile("Error in the conf file : max_client_body_size : invalid character after the unit size");
	}
	if (_maxBodySize < 0 || _maxBodySize > (3 * 1048576))// limite est de 2MB environ
		throw ErrorConfigFile("Error in the conf file : max_body_client_size : size has to be between 0 and 3MB");
}

void	VirtualServer::parseErrorPages(std::istringstream& iss)
{
	std::string	code;
	std::vector <int> codeList;
	int	errorCode;

	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : error_page : missing informations1");
	errorCode = parseErrorCode(code);
	codeList.push_back(errorCode);
	while ((iss >> code) && code.find_first_not_of("0123456789") == std::string::npos)
	{
		// std::cerr << "code line : " << code << "\n";
		errorCode = parseErrorCode(code);
		codeList.push_back(errorCode);
	}
	if (code.empty())
		throw ErrorConfigFile("Error in the conf file : error_page : missing informations2");
	// parsePathErrorPage(code);
	if (code[0] != '/' && code.find("..") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : error_page : wrong path");
	if (iss >> code)
		throw ErrorConfigFile("Error in the conf file : error_page : wrong content");
	for (size_t i = 0; i < codeList.size(); i++)
		_errorPages[codeList[i]] = code;
	
	// for (size_t i = 0; i < codeList.size(); i++)
	// 	std::cerr << "codeList[" << i << "] = " << codeList[i] << "\n";
	// std::cerr << "_errorPage : \n";
    // for (std::map<int, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {
    //     std::cout << it->first << " => " << it->second << "\n";
    // }
	// std::cerr << "\n";
}

int	VirtualServer::parseErrorCode(std::string& code)
{
	size_t	index = code.find_first_not_of("0123456789");
	std::string	path;
	if (index == std::string::npos) // pas d'autres caracteres que 0123456789
	{
		int errorCode = strtol(code.c_str(), NULL, 10);
		if (errorCode < 100 || errorCode > 599)
			throw ErrorConfigFile("Error in the conf file : error_page : wrong code");
		return (errorCode);
	}
	else
		throw ErrorConfigFile("Error in the conf file : error_page : wrong code");
}

void	VirtualServer::parseReturn(std::istringstream& iss)
{
	std::string	code;
	std::vector <int> codeList;
	int	errorCode;

	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : return : missing informations");
	errorCode = parseCodeReturn(code);
	codeList.push_back(errorCode);
	while ((iss >> code) && code.find_first_not_of("0123456789") == std::string::npos)
	{
		errorCode = parseCodeReturn(code);
		codeList.push_back(errorCode);
	}
	if (code.empty())
		throw ErrorConfigFile("Error in the conf file : return : missing informations");
	if (code[0] != '/' && code.find("..") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : return : wrong path");
	if (iss >> code)
		throw ErrorConfigFile("Error in the conf file : return : wrong content");
	for (size_t i = 0; i < codeList.size(); i++)
		_returnPages[codeList[i]] = code;
}

int	VirtualServer::parseCodeReturn(std::string& code)
{
	size_t	index = code.find_first_not_of("0123456789");
	std::string	path;
	if (index == std::string::npos)
	{
		int	errorCode = strtol(code.c_str(), NULL, 10);
		if (errorCode < 300 || errorCode > 308)
			throw ErrorConfigFile("Error in the conf file : return : wrong code");
		return (errorCode);
	}
	else
		throw ErrorConfigFile("Error in the conf file : return : wrong code");
}

void	VirtualServer::parseIndex(std::istringstream& iss)
{
	std::string	index;
	if (!(iss >> index))
		throw ErrorConfigFile("Error in the conf file : index : missing information");
	// A COMPLETER AVEC CAS D'ERREUR ?
	_indexPages.push_back(index);
	while (iss >> index)
	{
		// A COMPLETER AVEC CAS D'ERREUR ?
		_indexPages.push_back(index);
	}
}

void	VirtualServer::parseDefaultServer(std::istringstream& iss)
{
	std::string	string;

	_defaultVS = true;
	if (iss >> string)
		throw ErrorConfigFile("Error in the conf file : default_server : wrong content");
}

void	VirtualServer::connectVirtualServers()
{

		// struct sockaddr_in sa;
		int socket_fd;
		int status;

		// Create the socket
		socket_fd = socket(_address.sin_family, SOCK_STREAM, 0);
		if (socket_fd == -1)
			callException(-1);
		fcntl(socket_fd, F_SETFL, O_NONBLOCK);
		printf("[Server] Created server socket fd: %d\n", socket_fd);

		// Bind socket to address and port
		int opt = 1;
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, 4);
		status = bind(socket_fd, (struct sockaddr *)&_address, sizeof _address);
		if (status != 0)
		{
			callException(-1);
		}
		printf("[Server] Bound socket address ip = %s port %d\n", _ip.c_str(), _port);
		status = listen(socket_fd, 10); // A MODIF
		if (status != 0)
			callException(-1);
		_socketfd = socket_fd;
}

void	VirtualServer::checkNecessaryLine()
{
	if (_listenState == false)
		throw ErrorConfigFile("Error in the conf file : listen is missing");
	// bool	_listenState;
	// bool	_serverNameState;
	// bool	_rootState;
	// bool	_returnState;
	for(std::map<std::string, Location>::iterator loc = _location.begin(); loc != _location.end(); loc++)
	{

		std::map<std::string, std::vector<std::string> >::iterator configLoc = (*loc).second.getConfigLocation().find("rootDir");

		if (configLoc == (*loc).second.getConfigLocation().end() && (*loc).second.getReturn().empty() 
		&& _returnPages.empty() && _rootDir.empty())
			throw ErrorConfigFile("Error in the conf file : location : root missing");
	}
}

// GETTERS / SETTERS -------------------------------------------------------------------- //

int&	VirtualServer::getPort()
{
	return (_port);
}

void	VirtualServer::setPort(int port)
{
	_port = port;
}

int&	VirtualServer::getFd()
{
	return (_socketfd);
}

void	VirtualServer::setfd(int fd)
{
	_socketfd = fd;
}

size_t	VirtualServer::getMaxBodySize()
{
	return _maxBodySize;
}

int	&VirtualServer::getIsBind()
{
	return _isBind;
}

void	VirtualServer::setIsBind(int bind)
{
	_isBind = bind;
}

void		VirtualServer::setIp(std::string ip)
{
	_ip = ip;
}

std::string	&VirtualServer::getServerName()
{
	return _serverName;
}

void	VirtualServer::setToErase(bool erase)
{
	_toErase = erase;
}

bool	&VirtualServer::getToErase()
{
	return _toErase;
}

void	VirtualServer::setDefaultVS(bool value)
{
	_defaultVS = value;
}

bool	&VirtualServer::getDefaultVS()
{
	return _defaultVS;
}

int	&VirtualServer::getSocketFd()
{
	return _socketfd;
}

void	VirtualServer::setIndex(int i)
{
	_index = i;
}

int	&VirtualServer::getIndex()
{
	return _index;
}

std::string	&VirtualServer::getIP()
{
	return _ip;
}

bool	&VirtualServer::getPortByDefault()
{
	return _portByDefault;
}

std::map<std::string, Location>	&VirtualServer::getLocations()
{
	return _location;
}

std::string &VirtualServer::getRoot()
{
	return _rootDir;
}

std::vector<std::string> &VirtualServer::getIndexPage()
{
	return _indexPages;
}

bool	&VirtualServer::getAutoIndex()
{
	return _indexOnOff;
}

std::map<int, std::string>	&VirtualServer::getErrorPages()
{
	return _errorPages;
}

std::map<int, std::string>	&VirtualServer::getReturnPages()
{
	return _returnPages;
}