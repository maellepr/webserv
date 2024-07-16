#include "../includes/webserv.hpp"

// Global Data ------------------------------------------------------------------------ //

extern std::map<StatusCode, std::string>	STATUS_MESSAGES;
extern std::map<std::string, std::string>	CONTENT_TYPES;
extern bool noSignal;

void	fillStatusMsg()
{
	STATUS_MESSAGES[STATUS_NONE] = "OK"; // A MODIF
	STATUS_MESSAGES[STATUS_OK] = "OK"; // A MODIF
	STATUS_MESSAGES[STATUS_MOVED_PERMANENTLY] = "Moved Permanently";
	STATUS_MESSAGES[STATUS_FORBIDDEN] = "Forbidden";
	STATUS_MESSAGES[STATUS_BAD_REQUEST] = "Bad Request";
	STATUS_MESSAGES[STATUS_NOT_FOUND] = "Not Found";
	STATUS_MESSAGES[STATUS_REQUEST_TIMEOUT] = "Request Time-out";
	STATUS_MESSAGES[STATUS_PAYLOAD_TOO_LARGE] = "Request Entity Too Large";
	STATUS_MESSAGES[STATUS_URI_TOO_LONG] = "Request-URI Too Long";
	STATUS_MESSAGES[STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
	STATUS_MESSAGES[STATUS_INTERNAL_SERVER_ERROR] = "Internal Server Error";
	STATUS_MESSAGES[STATUS_NOT_IMPLEMENTED] = "Not Implemented";
	STATUS_MESSAGES[STATUS_HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version not supported";
}

void		fillContentTypes()
{
	CONTENT_TYPES["html"] = "text/html";
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
