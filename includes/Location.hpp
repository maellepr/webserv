#ifndef LOCATION_HPP
# define LOCATION_HPP

#include "webserv.hpp"

class Location
{
	public:
		Location();
		~Location();

	private:
		std::map<std::string, std::vector<std::string> >	_configLocation;
		// root
		// methods
		// dir-listing / default-uri

};

#endif