#include "htmlPage.hpp"

htmlPage::htmlPage() {};

htmlPage::htmlPage(const std::string& name) {
	_name = name;
	_webDirectory = "./www/";
}

htmlPage::~htmlPage() {};

void	htmlPage::setHeader() {
	_header += "HTTP/1.1 200 OK\n";
	_header += "Server: webserv\n";
	_header += "Content-Type: text/html\n";
	_header += "Connection: close\n\n";
}

std::string	htmlPage::getHeader() { return(_header); }	

//	Determine error return value
void	htmlPage::readFile(int file, size_t st_size) {
	char	buffer[st_size];
	int	ret = 0;

	if ((ret = read(file, buffer, st_size)) < 0)
		std::cout << "error read" << std::endl;
	// -1 because all byte have been read
	buffer[ret - 1] = '\0';
	_packet += buffer;
	close(file);
}

void	htmlPage::pullBody() {
	int		file = 0;
	std::string	path;
	struct stat	st;

	path = _webDirectory + _name;
	if ((file = open(path.c_str(), O_RDONLY)) < 0) {
		std::cout << strerror(errno) << std::endl;
		// default page
	}
	_packet += _header;
	if (fstat(file, &st) < 0)
		std::cout << strerror(errno);
	readFile(file, st.st_size);
	setDataSize();
}

void	htmlPage::setData() {
	pullBody();
}

char*	htmlPage::getData() {
	return (const_cast<char *>(_packet.c_str()));
}

size_t	htmlPage::getDataSize() { return (_dataSize); }

void	htmlPage::setDataSize() { _dataSize = _packet.size(); }


