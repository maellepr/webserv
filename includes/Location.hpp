#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "webserv.hpp"

class Location
{
	public:
		Location();
		~Location();

		void	setEqualModifier(bool state);
		bool	&getEqualModifier();

		void	parseLocation(std::istream& file);
		void	parseLocationOne(std::istringstream& iss, std::string keyword);
	
	private:
		std::map<std::string, std::vector<std::string> >	_configLocation;
		// root
		// methods
		// dir-listing / default-uri

		bool	_equalModifier;

};

#endif