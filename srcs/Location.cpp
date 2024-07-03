#include "../includes/Location.hpp"

Location::Location()
{
	_equalModifier = false;
}

Location::~Location()
{
}

void	Location::setEqualModifier(bool state)
{
	_equalModifier = state;
}

void	Location::parseLocation(std::istream& file)
{
	std::string	line;

	while (std::getline(file, line))
	{
		std::cerr << "line === " << line << "\n";
		std::istringstream iss(line);
		std::string	keyword;

		if (!(iss >> keyword))
			throw ErrorConfigFile("Error in the conf file : location : missing content4");
		if (line == "root")
		{
			parseLocationOne(iss, "root");
		}
		else if (line == "cgi")
		{
			continue ;
		}
		else if (line == "error_page")
		{
			continue ;
		}
		else if (line == "index")
		{
			continue ;
		}
		else if (line == "auto index")
		{
			continue ;
		}
		else if (line == "allow_methods")
		{
			continue ;
		}
		else if (line == "return")
		{
			continue ;
		}
		else if (line == "upload dir")
		{
			continue ;
		}
		else if (line == "	}")
			return ;
		else
			throw ErrorConfigFile("Error in the conf file : location : wrong content");
	}
}

void	Location::parseLocationOne(std::istringstream& iss, std::string keyword)
{
	std::string	root;
	std::vector<std::string>	content;

	if (!(iss >> root))
		throw ErrorConfigFile("Error in the conf file : location : root missing content");
	content.push_back(root);
	while (iss >> root)
		content.push_back(root);
	_configLocation[keyword] = content;
}