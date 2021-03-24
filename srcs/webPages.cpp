#include "webPages.hpp"

webPages::webPages() {
	/* If no directory specified -> './www/' */
	_webDirectory = "./www/";
}

webPages::~webPages() {}

void	webPages::setHeader(/* Parsing send info */) {
	/* default header */
        _header += "HTTP/1.1 200 OK\n";
	_header += "Server: webserv\n";
	_header += "Content-Type: text/html\n";
	_header += "Connection: close\n\n";
}

//	Malloc set errno ?
bool	webPages::readFile(int file) {
	char*	buffer = NULL;
	int	ret = 0;

	if (!(buffer = (char*)malloc(50)))
		return (false);
	while ((ret = read(file, buffer, 49)) > 0) {
		buffer[ret] = '\0';
		_page += buffer;
	}
	free(buffer);
	return (true);
}

/*	If 404page is removed in web_directory, nohting will be send */
void	webPages::setDefaultPage() {
	int	file = 0;

	std::string path = _webDirectory + "404.html";
	if ((file = open(path.c_str(), O_RDWR | O_RDONLY)) < 0) {
		_page += "404";
		return;
	}
	else {
		readFile(file);
		close(file);
	}
}

void	webPages::setPages(const std::string& name) {
	int	file = 0;

	_page += _header;
	std::string path = _webDirectory + name;
	if ((file = open(path.c_str(), O_RDWR | O_RDONLY)) < 0) {
		setDefaultPage();
	}
	else {
		readFile(file);
		close (file);
	}
}

char*	webPages::getPages() {
	return (const_cast<char*>(_page.c_str()));
}

size_t	webPages::getLength() {
	return (_page.length());
}
