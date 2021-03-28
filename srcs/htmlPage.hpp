/*
**	This class handle the buffer as a std::string
**	and return the data as char*
**	
*/

#include "IresponsePacket.hpp"

#include <iostream> // cout

#include <sys/types.h> // fstat
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h> // open

#include <errno.h> // errno
#include <string.h> // strerror

class htmlPage : protected IresponsePacket {
	
	public:
		htmlPage(const std::string &name);
		virtual ~htmlPage();

		void		setHeader();
		std::string	getHeader();
		/*	This function add the requested body (aka file)
			to the _packet */
		void		pullBody();
		void		readFile(int file, size_t st_size);

		void		setData();
		char*		getData();
		size_t		getDataSize();
		void		setDataSize();

	private:
		std::string	_name;
		std::string	_packet;
		std::string	_header;
		std::string	_webDirectory;
		htmlPage();

		/*virtual void	setData() = 0;
		virtual char	*getData() = 0;
		virtual size_t	getDataSize() = 0;
		virtual void	setDataSize() = 0;*/


}; // htmlPage
