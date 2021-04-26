#include "cgi.hpp"

namespace Webserv {

CgiHandler::CgiHandler(const Http::Request &request, const Configuration::location *config):
_body(request.get_body())
{
	(void)config;
	this->_initEnv(request, *config);
}

CgiHandler::CgiHandler(CgiHandler const &src) {
	if (this != &src) {
		this->_body = src._body;
		this->_env = src._env;
	}
	return ;
}

CgiHandler::~CgiHandler(void) {
	return ;
}

CgiHandler	&CgiHandler::operator=(CgiHandler const &src) {
	if (this != &src) {
		this->_body = src._body;
		this->_env = src._env;
	}
	return *this;
}

void		CgiHandler::_initEnv(const Http::Request &request, const Configuration::location &config) {
	Http::HeaderMap	headers = request.get_headers();
	if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
		this->_env["AUTH_TYPE"] = headers["Authorization"];

	// std::cout << "uri: " << request.get_uri() << std::endl;
	this->_env["REDIRECT_STATUS"] = "200"; //Security needed to execute php-cgi
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->_env["SCRIPT_NAME"] = "YoupiBanane/directory/youpi.bla";
	this->_env["SCRIPT_FILENAME"] = "YoupiBanane/directory/youpi.bla";
	this->_env["REQUEST_METHOD"] = request.get_method();
	this->_env["CONTENT_LENGTH"] = Utils::itoa(request.get_body().length());
	this->_env["CONTENT_TYPE"] = headers["Content-Type"];
	this->_env["PATH_INFO"] = request.get_uri(); //might need some change, using config path/contentLocation
	this->_env["PATH_TRANSLATED"] = request.get_uri(); //might need some change, using config path/contentLocation
	this->_env["QUERY_STRING"] = "";
	this->_env["REMOTE_ADDR"] = "127.0.0.1";
	this->_env["REMOTE_IDENT"] = headers["Authorization"];
	this->_env["REMOTE_USER"] = headers["Authorization"];
	this->_env["REQUEST_URI"] = request.get_uri();
	if (headers.find("Hostname") != headers.end())
		this->_env["SERVER_NAME"] = headers["Hostname"];
	else
		this->_env["SERVER_NAME"] = this->_env["REMOTE_ADDR"];
	this->_env["SERVER_PORT"] = "8080";
	this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->_env["SERVER_SOFTWARE"] = "Weebserv/1.0";

	for (Http::HeaderMap::const_iterator it = request.get_headers().begin();
		it != request.get_headers().end(); ++it) {
			std::string s = it->first;
			std::transform(s.begin(), s.end(), s.begin(), toupper);
			_env[s] = it->second;
	}
	if (_env.find("X-SECRET-HEADER-FOR-TEST") != _env.end()) {
		this->_env["HTTP_X_SECRET_HEADER_FOR_TEST"] = "1";
		_env.erase("X-SECRET-HEADER-FOR-TEST");
	}
	// for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
		// std::cout << (*it).first << " = " << (*it).second << std::endl;
}

char					**CgiHandler::_getEnvAsCstrArray() const {
	char	**env = new char*[this->_env.size() + 1];
	int	j = 0;
	for (std::map<std::string, std::string>::const_iterator i = this->_env.begin(); i != this->_env.end(); i++) {
		std::string	element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		env[j] = strcpy(env[j], (const char*)element.c_str());
		j++;
	}
	env[j] = NULL;
	return env;
}

void		CgiHandler::executeCgi(const std::string& scriptName, Http::Response &response) {
	pid_t		pid;
	int			saveStdin;
	int			saveStdout;
	char		**env;
	std::string	newBody;

	try {
		env = this->_getEnvAsCstrArray();
	}
	catch (std::bad_alloc &e) {
		std::cerr << e.what() << std::endl;
	}

	// SAVING STDIN AND STDOUT IN ORDER TO TURN THEM BACK TO NORMAL LATER
	saveStdin = dup(STDIN_FILENO);
	saveStdout = dup(STDOUT_FILENO);

	FILE	*fIn = tmpfile();
	FILE	*fOut = tmpfile();
	long	fdIn = fileno(fIn);
	long	fdOut = fileno(fOut);
	int		ret = 1;

	write(fdIn, _body.c_str(), _body.size());
	lseek(fdIn, 0, SEEK_SET);

	pid = fork();
	response.unset_body();
	if (pid == -1)
	{
		response.set_status_code("500");
		return ;
	}
	else if (!pid)
	{
		char * const * nll = NULL;
		dup2(fdIn, STDIN_FILENO);
		dup2(fdOut, STDOUT_FILENO);
        // std::cerr << "exec with " << scriptName.c_str() << std::endl;
		execve(scriptName.c_str(), nll, env);
		response.set_status_code("500");
		std::cerr << "Execve crashed." << std::endl;
		write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
	}
	else
	{
		char* buffer = new char[1000000];
		// char	buffer[1000000];

		waitpid(-1, NULL, 0);
		lseek(fdOut, 0, SEEK_SET);

		ret = 1;
		while (ret > 0)
		{
			memset(buffer, 0, 1000000);
			ret = read(fdOut, buffer, 1000000 - 1);
			// buffer[ret] = 0;
			newBody += buffer;
		}
		delete[] buffer;
	}

	dup2(saveStdin, STDIN_FILENO);
	dup2(saveStdout, STDOUT_FILENO);
	fclose(fIn);
	fclose(fOut);
	close(fdIn);
	close(fdOut);
	close(saveStdin);
	close(saveStdout);

	for (size_t i = 0; env[i]; i++)
		delete[] env[i];
	delete[] env;

	if (!pid)
		exit(0);

	// std::cout << "b: " <<  newBody.size() << std::endl;
	size_t		i = 0;
	size_t		j = - 2;
	response.set_status_code("200");
	while (newBody.find("\r\n\r\n", i) != std::string::npos || newBody.find("\r\n", i) == i)
	{
		std::string	str = newBody.substr(i, newBody.find("\r\n", i) - i);
		if (str.find("Status: ") == 0)
			response.set_status_code(str.substr(8, 3).c_str());
		else if (str.find("Content-type: ") == 0)
			response.append_header("Content-Type", str.substr(14, str.size()));
		i += str.size() + 2;
	}
	while (newBody.find("\r\n", j) == j)
		j -= 2;
	newBody = newBody.substr(i, j - i);
	response.set_body(newBody);
	return ;
}

};