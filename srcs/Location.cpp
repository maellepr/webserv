#include "../includes/Location.hpp"

Location::Location()
{
	_equalModifier = false;
}

Location::Location(std::map<int, std::string>& returnPages)
{
	_equalModifier = false;
	if (!(returnPages.empty()))
	{
		std::map<int, std::string>::iterator rp = returnPages.begin();
		int	code = rp->first;
		std::string	page = rp->second;
		_returnPagesLocation[code] = page;
	}
}

Location::~Location()
{
}

void	Location::setEqualModifier(bool state)
{
	_equalModifier = state;
}

bool	&Location::getEqualModifier()
{
	return _equalModifier;
}

void	Location::parseLocation(std::istream& file)
{
	std::string	line;

	while (std::getline(file, line))
	{
		// std::cerr << "line === " << line << "\n";
		std::istringstream iss(line);
		std::string	keyword;

		if (!(iss >> keyword))
			throw ErrorConfigFile("Error in the conf file : location : missing content4");
		// std::cerr << "keyword === " << keyword << "\n";
		if (keyword == "root")
		{
			parseLocation(iss, "root");
		}
		else if (keyword == "cgi")
		{
			parseLocation(iss, "cgi");
		}
		else if (keyword == "index")
		{
			parseLocation(iss, "index");
		}
		else if (keyword == "auto_index")
		{
			parseLocation(iss, "auto_index");
		}
		else if (keyword == "methods")
		{
			parseLocation(iss, "methods");
		}
		else if (keyword == "return")
		{
			parseLocation(iss, "return");
		}
		else if (keyword == "upload_dir")
		{
			parseLocation(iss, "upload_dir");
		}
		else if (keyword == "error_page")
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
			std::cerr << "parseLocation keyword : " << keyword << "\n";
			throw ErrorConfigFile("Error in the conf file : location : wrong content 5");
		}
	}
	// std::cerr << "_config Location : \n";
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
	// std::cerr << "\n\n";
}

void	Location::parseLocation(std::istringstream& iss, std::string keyword)
{
	std::string	word;
	std::vector<std::string>	content;

	if (!(iss >> word))
		throw ErrorConfigFile("Error in the conf file : location : root missing content");
	content.push_back(word);
	while (iss >> word)
		content.push_back(word);
	_configLocation[keyword] = content;
}

void	Location::parseLocationErrorPage(std::istringstream& iss)
{
	std::string	code;
	std::vector <int> codeList;
	int	errorCode;

	if (!(iss >> code))
		throw ErrorConfigFile("Error in the conf file : location : error_page : missing informations1");
	errorCode = parseErrorCode(code);
	codeList.push_back(errorCode);
	while ((iss >> code) && code.find_first_not_of("0123456789") == std::string::npos)
	{
		errorCode = parseErrorCode(code);
		codeList.push_back(errorCode);
	}
	if (code.empty())
		throw ErrorConfigFile("Error in the conf file : location : error_page : missing informations2");
	// parsePathErrorPage(code);
	if (code[0] != '/' && code.find("..") != std::string::npos)
		throw ErrorConfigFile("Error in the conf file : location : error_page : wrong path");
	if (iss >> code)
		throw ErrorConfigFile("Error in the conf file : location : error_page : wrong content");
	for (size_t i = 0; i < codeList.size(); i++)
	{
		_errorPages[codeList[i]] = code;
	}
}

int	Location::parseErrorCode(std::string& code)
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

void	Location::setPrefix(std::string prefix)
{
	_prefix = prefix;
}