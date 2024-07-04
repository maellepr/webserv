#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "webserv.hpp"

class Location
{
	public:
		Location();
		~Location();

		void	setEqualModifier(bool state);

		void	parseLocation(std::istream& file);
		void	parseLocation(std::istringstream& iss, std::string keyword);
		void	parseLocationErrorPage(std::istringstream& iss);
		int		parseErrorCode(std::string& code);

	private:
		std::map<std::string, std::vector<std::string> >	_configLocation;
		std::map<int, std::string> 			_errorPages;
		// root
		// methods
		// dir-listing / default-uri

		bool	_equalModifier;

};

#endif