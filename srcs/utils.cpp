#include "../includes/webserv.hpp"

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
		throw ErrorConfigFile("Error the path is not a directory");
		// std::cerr << "Error : " << path << " is not a directory\n";
		// return false;
	}
	return ;
}