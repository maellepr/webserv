#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "webserv.hpp"
// #include "VirtualServer.hpp"

// class VirtualServer;
class Location
{
	public:
		Location();
		Location(std::map<int, std::string>& returnPages, VirtualServer& vs);
		~Location();

		void	setEqualModifier(bool state);
		bool	&getEqualModifier();

		std::map<int, std::string>	&getReturn();

		std::string	&getPrefix();

		void	setPrefix(std::string prefix);

		std::map<std::string, std::vector<std::string> >	&getConfigLocation();

		std::map<int, std::string>	&getErrorPages();

		void	parseLocation(std::istream& file);
		void	parseLocation(std::istringstream& iss, std::string keyword);
		void	parseLocationErrorPage(std::istringstream& iss);
		int		parseErrorCode(std::string& code);
		void	parseReturnPage(std::istringstream& iss);
		int		parseReturnCode(std::string& code);

	private:
		std::string					_prefix;
		std::map<std::string, std::vector<std::string> >	_configLocation;
		std::map<int, std::string>	_errorPages;
		std::map<int, std::string>	_returnPageLocation;
		// root
		// methods
		// dir-listing / default-uri

		bool	_equalModifier;

};

#endif