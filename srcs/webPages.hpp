#pragma once

#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h> // read

#include <errno.h> // perror
#include <stdio.h>

class webPages
{
	public:
		webPages();
		~webPages();
		void	setHeader();
		void	setPages(const std::string& name);
		char*	getPages();
		size_t	getLength();

	private:
		std::string	_page;
		std::string	_header;
		std::string	_webDirectory;

		bool	readFile(int file);
		void	setDefaultPage();

}; // class webPages
