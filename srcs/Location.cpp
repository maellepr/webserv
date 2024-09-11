#include "../includes/Location.hpp"

Location::Location()
{
	// _equalModifier = false;
}

Location::Location(std::map<int, std::string>& returnPages, VirtualServer& vs, bool serverActAsLocation)
{
	_equalModifier = false;
	_serverActAsLocation = serverActAsLocation;

	if (!(returnPages.empty()))
	{
		std::map<int, std::string>::iterator rp = returnPages.begin();
		int	code = rp->first;
		std::string	page = rp->second;
		_returnPageLocation[code] = page;
	}
	std::vector<std::string> root;
	if (!(vs.getRoot().empty()))
	{
		root.push_back(vs.getRoot());
		_configLocation["rootDir"] = root;
	}
	std::vector<std::string> indexPages;
	indexPages = vs.getIndexPage();
	if (!(indexPages.empty()))
		_configLocation["index"] = indexPages;

	std::vector<std::string> autoIndex;
	bool	aI = vs.getAutoIndex();
	if (aI == false)
		autoIndex.push_back("false");
	else if (aI == true)
		autoIndex.push_back("true");
	_configLocation["autoindex"] = autoIndex;

	_errorPages = vs.getErrorPages();

	_returnPageLocation = vs.getReturnPages();

	_returnState = false;
	_errorPageState = false;
}

Location::~Location()
{
}



void	Location::parseLocation(std::istream& file)
{
	std::string	line;

	while (std::getline(file, line))
	{
		// std::cerr << "line === " << line << "\n";
		std::istringstream iss(line);
		std::string	keyword;
		if (line.empty() || line == "\t\t")//
			continue ;
		if (!(iss >> keyword))
		{
			// std::cerr << "keyword = " << keyword << "\n";
			throw ErrorConfigFile("Error in the conf file : location : missing content");
		}

		if (keyword == "root" && _configLocation.find("root") == _configLocation.end())
		{
			parseLocation(iss, "rootDir");
		}
		else if (keyword == "cgi" && _configLocation.find("cgi") == _configLocation.end())
		{
			parseLocation(iss, "cgi");
		}
		else if (keyword == "index" && _configLocation.find("index") == _configLocation.end())
		{
			parseLocation(iss, "index");
		}
		else if (keyword == "auto_index" && _configLocation.find("auto_index") == _configLocation.end())
		{
			parseLocation(iss, "auto_index");
		}
		else if (keyword == "methods" && _configLocation.find("methods") == _configLocation.end())
		{
			parseLocation(iss, "methods");
		}
		else if (keyword == "return" && _returnState == false)
		{
			parseReturnPage(iss);
		}
		else if (keyword == "upload_dir" && _configLocation.find("upload_dir") == _configLocation.end())
		{
			parseLocation(iss, "upload_dir");
		}
		else if (keyword == "error_page" && _errorPageState == false)
		{
			parseLocationErrorPage(iss);
		}
		else if (keyword == "#")
		{
			continue ;
		}
		else if (keyword == "}")
			break ;
		else
		{
			// std::cerr << "parseLocation keyword : " << keyword << "\n";
			throw ErrorConfigFile("Error in the conf file : location : wrong content");
		}
	}
	// std::cerr << "\n_config Location : \n";
	// std::cerr << "prefix : " << _prefix << "\n";
    // for (std::map<std::string, std::vector<std::string> >::iterator it = _configLocation.begin(); it != _configLocation.end(); it++) 
	// {
    //     std::cout << "Key: " << it->first << std::endl;
    //     for (std::vector<std::string>::iterator vecIt = it->second.begin(); vecIt != it->second.end(); ++vecIt) {
    //         std::cout << "    Value: " << *vecIt << std::endl;
    //     }
    // }
	// std::cerr << "_errorPage of Location :\n"; 
    // for (std::map<int, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); it++) {
    //     std::cout << it->first << " => " << it->second << "\n";
    // }
	// std::cerr << "_returnPage of Location :\n"; 
    // for (std::map<int, std::string>::iterator ret = _returnPageLocation.begin(); ret != _returnPageLocation.end(); ret++) {
    //     std::cout << ret->first << " => " << ret->second << "\n";
    // }
	// std::cerr << "equalmodifier = " << _equalModifier << "\n"; 
	// std::cerr << "\n\n";
}

void	Location::parseLocation(std::istringstream& iss, std::string keyword)
{
	std::string	word;
	std::vector<std::string>	content;

	if (!(iss >> word))
		throw ErrorConfigFile("Error in the conf file : location : missing content");
	std::map<std::string, std::vector<std::string> >::iterator confLoc = _configLocation.find(keyword);
	if (confLoc != _configLocation.end())// keyword existe
		_configLocation.erase(keyword);
	content.push_back(word);
	if (keyword == "cgi" || keyword == "rootDir" || keyword == "auto_index")
	{
		if (iss >> word)
			throw ErrorConfigFile("Error in the conf file : location : wrong content");
	}
	while (iss >> word)
		content.push_back(word);
	_configLocation[keyword] = content;
}

void	Location::parseReturnPage(std::istringstream& iss)
{
	if (!(_returnPageLocation.empty()))
		return ;
	std::string	code;
	int	c;
	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : location : return missing content");
	c = parseReturnCode(code);
	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : location : return missing content");
	if (code[0] != '/' && code.find("..") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : location : return : wrong path");
	if (iss >> code)
		throw ErrorConfigFile("Error in the conf file : location : return : wrong content");
	_returnPageLocation[c] = code;
	_returnState = true;
}

int	Location::parseReturnCode(std::string& code)
{
	size_t	index = code.find_first_not_of("0123456789");
	std::string	path;
	if (index == std::string::npos) // pas d'autres caracteres que 0123456789
	{
		int errorCode = strtol(code.c_str(), NULL, 10);
		if (errorCode < 300 || errorCode > 308)
			throw ErrorConfigFile("Error in the conf file : location : return : wrong code");
		return (errorCode);
	}
	else
		throw ErrorConfigFile("Error in the conf file : location : return : wrong code");
}


void	Location::parseLocationErrorPage(std::istringstream& iss)
{
	std::string	code;
	std::vector <int> codeList;
	int	errorCode;

	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : location : error_page : missing informations");
	if (!(_errorPages.empty()))
		_errorPages.clear();
	errorCode = parseErrorCode(code);
	codeList.push_back(errorCode);
	while ((iss >> code) && code.find_first_not_of("0123456789") == std::string::npos)
	{
		errorCode = parseErrorCode(code);
		codeList.push_back(errorCode);
	}
	if (code.empty())
		throw ErrorConfigFile("Error in the conf file : location : error_page : missing informations");
	// parsePathErrorPage(code);
	if (code[0] != '/' && code.find("..") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : location : error_page : wrong path");
	if (iss >> code)
		throw ErrorConfigFile("Error in the conf file : location : error_page : wrong content");
	for (size_t i = 0; i < codeList.size(); i++)
	{
		_errorPages[codeList[i]] = code;
	}
	_errorPageState = true;
}

int	Location::parseErrorCode(std::string& code)
{
	size_t	index = code.find_first_not_of("0123456789");
	std::string	path;
	if (index == std::string::npos) // pas d'autres caracteres que 0123456789
	{
		int errorCode = strtol(code.c_str(), NULL, 10);
		if (errorCode < 100 || errorCode > 599)
			throw ErrorConfigFile("Error in the conf file : location : error_page : wrong code");
		return (errorCode);
	}
	else
		throw ErrorConfigFile("Error in the conf file : location : error_page : wrong code");
}

void	Location::setEqualModifier(bool state)
{
	_equalModifier = state;
}

bool	&Location::getEqualModifier()
{
	return _equalModifier;
}

std::string	&Location::getPrefix()
{
	return _prefix;
}

void	Location::setPrefix(std::string prefix)
{
	_prefix = prefix;
}

std::map<int, std::string>	&Location::getReturn()
{
	return _returnPageLocation;
}

std::map<std::string, std::vector<std::string> >	&Location::getConfigLocation()
{
	return _configLocation;
}

std::map<int, std::string> &Location::getErrorPages()
{
	return _errorPages;
}

bool	&Location::getServerActAsLocation()
{
	return _serverActAsLocation;
}