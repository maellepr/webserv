#include "../includes/webserv.hpp"

// Global Data ------------------------------------------------------------------------ //

extern std::map<StatusCode, std::string>	STATUS_MESSAGES;
extern std::map<std::string, std::string>	CONTENT_TYPES;
extern std::map<unsigned char, int>			HEXA_BASE;
extern bool noSignal;

void	fillStatusMsg()
{
	STATUS_MESSAGES[STATUS_NONE] = "OK"; // A MODIF
	STATUS_MESSAGES[STATUS_OK] = "OK"; // A MODIF
	STATUS_MESSAGES[STATUS_CREATED] = "Created";
	STATUS_MESSAGES[STATUS_NO_CONTENT] = "No Content";
	STATUS_MESSAGES[STATUS_MULTIPLE_CHOICES] = "Status Multiple Choices";
	STATUS_MESSAGES[STATUS_MOVED_PERMANENTLY] = "Moved Permanently";
	STATUS_MESSAGES[STATUS_FOUND] = "Status Found";
	STATUS_MESSAGES[STATUS_SEE_OTHER] = "Status See Other";
	STATUS_MESSAGES[STATUS_NOT_MODIFIED] = "Status Not Modified";
	STATUS_MESSAGES[STATUS_USE_PROXY] = "Status Use Proxy";
	STATUS_MESSAGES[STATUS_SWITCH_PROXY] = "Status Switch Proxy";
	STATUS_MESSAGES[STATUS_TEMPORARY_REDIRECT] = "Status Temporary Redirect";
	STATUS_MESSAGES[STATUS_PERMANENT_REDIRECT] = "Status Permanent Redirect";
	STATUS_MESSAGES[STATUS_FORBIDDEN] = "Forbidden";
	STATUS_MESSAGES[STATUS_BAD_REQUEST] = "Bad Request";
	STATUS_MESSAGES[STATUS_NOT_FOUND] = "Not Found";
	STATUS_MESSAGES[STATUS_METHOD_NOT_ALLOWED] = "Method Not Allowed";
	STATUS_MESSAGES[STATUS_REQUEST_TIMEOUT] = "Request Time-out";
	STATUS_MESSAGES[STATUS_PAYLOAD_TOO_LARGE] = "Request Entity Too Large";
	STATUS_MESSAGES[STATUS_URI_TOO_LONG] = "Request-URI Too Long";
	STATUS_MESSAGES[STATUS_UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
	STATUS_MESSAGES[STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
	STATUS_MESSAGES[STATUS_INTERNAL_SERVER_ERROR] = "Internal Server Error";
	STATUS_MESSAGES[STATUS_NOT_IMPLEMENTED] = "Not Implemented";
	STATUS_MESSAGES[STATUS_HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version not supported";
}

void	fillContentTypes()
{
	CONTENT_TYPES["html"] = "text/html";
}

void	fillHexaBase()
{
	HEXA_BASE['0'] = 0;
	HEXA_BASE['1'] = 1;
	HEXA_BASE['2'] = 2;
	HEXA_BASE['3'] = 3;
	HEXA_BASE['4'] = 4;
	HEXA_BASE['5'] = 5;
	HEXA_BASE['6'] = 6;
	HEXA_BASE['7'] = 7;
	HEXA_BASE['8'] = 8;
	HEXA_BASE['9'] = 9;
	HEXA_BASE['A'] = 10;
	HEXA_BASE['B'] = 11;
	HEXA_BASE['C'] = 12;
	HEXA_BASE['D'] = 13;
	HEXA_BASE['E'] = 14;
	HEXA_BASE['F'] = 15;
}

void	extension(const std::string & str, const std::string & ext)
{
    if (str.size() >= ext.size() && !str.compare(str.size() - ext.size(), ext.size(), ext))
		return ;	
	throw ErrorConfigFile("Error : wrong conf extension");
	// std::cerr << "Error : wrong conf extension\n";
	// return false;
    // int compare (size_t begin_pos, size_t len, const string& str) const;
    // return 0 -> they're equal
    // return <0 or >0 a character doesn't match or compared string is longer/shorter 
}

void	isDirectory(const std::string & path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)// cannot access path
	{
		throw ErrorConfigFile("Error : cannot access path or file");
		// std::cerr << "Error : cannot access path or file " << path << "\n";
		// return false;
	}
    if (S_ISDIR(info.st_mode) != 0)// is not a directory
	{
		throw ErrorConfigFile("Error the path is a directory");
		// std::cerr << "Error : " << path << " is not a directory\n";
		// return false;
	}
	return ;
}

bool	isPathADirectory(const std::string &path)
{
	struct stat buf;
	
	if (stat(path.c_str(), &buf) != 0)
	{
		std::cerr << "STAT FAILED : \n";
		std::cerr << strerror(errno) << std::endl;
		return (false);
	}
	return (S_ISDIR(buf.st_mode));
}

bool	isPathADRegularFile(const std::string &path)
{
	struct stat buf;
	
	if (stat(path.c_str(), &buf) != 0)
		return (false);
	return (S_ISREG(buf.st_mode));
}

// bool	isUriValid(const std::string &uri)
// {
// 	if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos)
// 		return (false);
// 	return true;
// }

bool	readContent(std::string &uri, std::string &content)
{
	std::cerr << "readContent 0\n";
	if (isPathADirectory(uri))
	{
		std::cerr << "readContent 1\n";
		return false;
	}
	std::ifstream	file(uri.c_str());
	if (!file.good())
	{
		std::cerr << "readContent 2\n";
		return false;
	}
	std::stringstream	buf;
	buf << file.rdbuf();
	content = buf.str();

	return true;
}

void	handleSignal(int signum)
{
	(void)signum;
	noSignal = false;
}

std::string	getAbsPath(std::string &path)
{
	// on compare le debut de path si c'est pas ./ on renvoie /
	if (!(path.size() >= 2 && path.compare(0,  2, "./")))
		return ("/");
	char	buffer[PATH_MAX];
	char *buf = getcwd(buffer, sizeof(buffer));
	if (buf)
		return (buf);
	return ("/");
}

char	**vectorStringToChar(std::vector<std::string> &vector)
{
	char** tab = new char*[vector.size() + 1];
	for (size_t i = 0; i < vector.size(); i++)
	{
		tab[i] = new char[vector[i].size() + 1];
		std::copy(vector[i].begin(), vector[i].end(), tab[i]);
		tab[i][vector[i].size()] = '\0';
	}
	tab[vector.size()] = NULL;
	return (tab);
}

void	freeChar(char **tab)
{
	int i = 0;
	while (tab[i])
	{
		delete [] tab[i];
		i++;
	}
	delete [] tab;
}

std::vector<unsigned char> vectorizeString(std::string s)
{
	return (std::vector<unsigned char>(s.begin(), s.end()));
}

std::string	stringifyVector(std::vector<unsigned char> v)
{
	return (std::string(v.begin(), v.end()));
}