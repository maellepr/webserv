#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "webserv.hpp"

class Location
{
	public:
		Location();
		Location(std::map<int, std::string>& returnPage);
		~Location();

		void	setEqualModifier(bool state);
		bool	&getEqualModifier();

		std::string	&getPrefix();

		void	setPrefix(std::string prefix);

		std::map<std::string, std::vector<std::string> >	&getConfigLocation();

		void	parseLocation(std::istream& file);
		void	parseLocation(std::istringstream& iss, std::string keyword);
		void	parseLocationErrorPage(std::istringstream& iss);
		int		parseErrorCode(std::string& code);

	private:
		std::string					_prefix;
		std::map<std::string, std::vector<std::string> >	_configLocation;
		std::map<int, std::string>	_errorPages;
		std::map<int, std::string>	_returnPagesLocation;
		// root
		// methods
		// dir-listing / default-uri

		bool	_equalModifier;

};

#endif