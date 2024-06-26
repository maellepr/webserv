#ifndef REQUEST_HPP
# define REQUEST_HPP

class Request
{
	public:
		Request();
		~Request();

		int	parse(char *buffer, size_t bytesRead);
	private:

};

#endif