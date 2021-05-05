#include "phpCgi.hpp"

/*
**	Error 500 (Bad Gateway interface) if incorrect exec cgi.
*/

/*
**	std::string method
**	std::string filepath
**	Http::Response response
*/

/*
**	Steps:
**		Check if executable is good
**		Set args
**		Set env var
**		Set pipe
**		Read from it in parents
**		Send it
*/

namespace Webserv {

phpCgi::phpCgi() {};

phpCgi::~phpCgi() {
	/*
	**	Delete env array
	*/
}

std::string	phpCgi::getScriptName(const std::string& filepath, const confLocation* location) {
	std::size_t start = filepath.find(location->_root, 0);
	start += location->_root.length();

	std::string ret = filepath.substr(start, filepath.length());
	std::size_t end = ret.find(".php", 0);
	ret = ret.substr(0, end + 4);
	ret.insert(0, "/");
	return (ret);
}

std::string	phpCgi::getPathInfo(const std::string& filepath) {
	std::string	ret;

	std::size_t start = filepath.find(".php", 0);
	start += 3;
	if (++start < filepath.length()) {
		if (filepath[start] == '?')
			return ("NO");
		std::size_t end = start;
		while (end <= filepath.length()) {
			end++;
			if (filepath[end] == '?')
				return (filepath.substr(start, end - start));
		}
		ret = filepath.substr(start, end - start);
	}
	else
		return ("NO");
	return (ret);
}

/*
**		root location in conf should be an absolute path
**			else there could be trouble with the use of getcwd function here
*/
std::string	phpCgi::getFullPath(const std::string& filepath, const confLocation* location) {
	std::string	ret;
	// Get script name and remove the first '/'
	std::string	name = getScriptName(filepath, location);
	//std::string name = "/action.php";
	name = name.substr(1, name.size());

	char *buf = new char[500];
	ret = getcwd(buf, 300);
	if (location->_root[0] == '.')
		ret += location->_root.substr(1, location->_root.length());
	else
		ret += location->_root;
	ret += name;
	return (ret);
}

std::string	phpCgi::getRequestUri(const std::string& filepath, const confLocation* location) {
	std::string	ret = filepath;
	std::size_t end = ret.find(".php", 0);
	ret = ret.substr(0, end + 4);
	ret.erase(0, location->_root.length());
	ret.insert(0, "/");
	return (ret);
}

std::string	phpCgi::getQueryString(const std::string& filepath) {
	std::size_t	start = filepath.find("?", 0);

	if (start != std::string::npos) {
		start++;
		std::string ret(filepath.begin() + start, filepath.end());
		return (ret);
	}
	else
		return ("");
}

char	*phpCgi::assembleMetaVar(const std::string& key, const std::string& value) {
	char	*ret = new char[key.size() + value.size() + 2];

	ret[0] = '\0';
	strcat(ret, key.c_str());
	ret = strcat(ret, "=");
	ret = strcat(ret, value.c_str());
	std::cout << "assemble: " << ret << std::endl;
	return (ret);
} // assembleMetaVar

int		phpCgi::getLenNumber(int n) {
	int	ret = 0;

	while (n > 0) {
		n /= 10;
		ret++;
	}
	return (ret);
}

std::string	phpCgi::ft_itos(const int& ipt) {
	int		n = ipt;
	int		nLen = getLenNumber(n);
	std::string	ret(nLen, '\0');

	nLen--;
	while (n > 0) {
		ret[nLen] = (n % 10) + 48;
		nLen--;
		n /= 10;
	}
	return (ret);

}

void	phpCgi::setEnvArray() {
	std::size_t len = _envVar.size();
	_envArray = new char*[len + 1];

	int	count = 0;
	envMap::iterator it = _envVar.begin();
	for (; it != _envVar.end(); it++) {
		_envArray[count++] = assembleMetaVar(it->first, it->second);
	}
	_envArray[count] = NULL;
}

/*
**	When no data for meta-variables, the meta shouldn't be set to "" (unbless CGI spec specifiy it)
**	because isset will detect it in php's script
*/

void	phpCgi::setEnvMap(
	const std::string& method,
	const confServer* server,
	const confLocation* location,
	const httpRequest& request,
	httpResponse& response,
	const std::string& filepath)
{
	const Http::HeaderMap header = request.get_headers();

	for (auto const &pair: header) {
		std::cout << "[" << pair.first << ":" << pair.second << "]" << std::endl;
	}
	std::cout << std::endl;
	std::cout << "filepath: " << filepath << std::endl;

	/*	AUTH_TYPE (Not empty if isn't set) To check	*/
	if (request.has_header("authorization"))
		_envVar["AUTH_TYPE"] = request.get_header_values("authorization");

	/*	CONTENT_LENGTH	*/
	if (request.has_header("content-length"))
		_envVar["CONTENT_LENGTH"] = request.get_header_values("content-length");

	/*	CONTENT_TYPE	*/
	if (request.has_header("content-type"))
		_envVar["CONTENT_TYPE"] = request.get_header_values("content-type");

	/*	GATEWAY_INTERFACE	*/
	_envVar["GATEWAY_INTERFACE"] = "CGI/1.1";

	/*	PATH_INFO */
	std::string pathInfo = getPathInfo(filepath);
	if (pathInfo != "NO")
		_envVar["PATH_INFO"] = pathInfo;

	/*	PATH_TRANSLATED (root if no PATH_INFO, else _root + PATH_INFO)	*/
	if (pathInfo != "NO")
		_envVar["PATH_TRANSLATED"] = location->_root + pathInfo;
	else
		_envVar["PATH_TRANSLATED"] = "";

	/*	QUERY_STRING everything after '?'
	**	contrary to what is said in the comment above the function, if there is no data,
	**	the QUERY_STRING variable MUST be defined as an empty string "". cf. CGI/1.1 specs
	*/
	_envVar["QUERY_STRING"] = getQueryString(filepath);

	/*	REMOTE_ADDR (IP address from client asking the page) */
	// TMP -> should be connect in the future
	_envVar["REMOTE_ADDR"] = "012.345.678.910";

	/*	REMOTE_IDENT, probably false	*/
	if (request.has_header("host"))
		_envVar["REMOTE_IDENT"] = request.get_header_values("host");

	/*	REMOTE_USER (if authorization is set)	*/
	if (request.has_header("authorization"))
		std::cout << "auth USER" << std::endl;

	/*	REQUEST_METHOD	*/
	_envVar["REQUEST_METHOD"] = method;

	/*	REQUEST_URI	*/
	_envVar["REQUEST_URI"] = getRequestUri(filepath, location);

	/*	SCRIPT_NAME	*/
	_envVar["SCRIPT_NAME"] = getScriptName(filepath, location);

	/*	SCRIPT_FILENAME	(needed to exec cgi)	*/
	_envVar["SCRIPT_FILENAME"] = getFullPath(filepath, location);

	/*	SERVER_NAME	*/
	_envVar["SERVER_NAME"] = server->_server_name;

	/*	SERVER_PORT	*/
	std::string port = ft_itos(server->_listen.first);
	_envVar["SERVER_PORT"] = port;

	/*	SERVER_PROTOCOL	*/
	_envVar["SERVER_PROTOCOL"] = "HTTP/1.1";

	/*	SERVER_SOFTWARE	*/
	_envVar["SERVER_SOFTWARE"] = "Webserv/1.1";

	/*	REDIRECT_STATUS (Needed to tell cgi that everything was handled good by the server)	*/
	_envVar["REDIRECT_STATUS"] = "200";

	/*	SERVER_ADDR	*/
	std::string ip = server->_listen.second;
	_envVar["SERVER_ADDR"] = ip;
} // setEnvMap

bool	phpCgi::headersAreOut(const std::string& body) {
	if (body.find("\r\n\r\n", 0) != std::string::npos)
		return (true);
	else
		return (false);
}

std::string	phpCgi::pullCode(std::string& value) {
	std::string	ret;
	std::size_t	start;

	start = value.find_first_of("0123456789");
	for (std::size_t i = start; value[i] >= '0' && value[i] <= '9'; i++)
			ret += value[i];
	return (ret);
}

std::string	phpCgi::cutHeaders(const std::string& body, httpResponse& response) {
	bool		status = false;
	std::size_t	start = 0;
	std::string name;
	std::string	value;

	while (body.find("\r\n\r\n") > start) {
		for (int i = start; body[i] != ':'; i++)
				name += body[i];
		start = body.find(':', start) + 2;
		for (int i = start; body[i] != '\r'; i++)
				value += body[i];
		start = body.find("\r\n", start) + 2;
		if (name == "Status") {
				response.set_status_code(pullCode(value));
				status = true;
		}
		else
				response.append_header(name, value);
		name.clear();
		value.clear();
	}
	if (!status)
			response.set_status_code("200");
	return(body.substr(body.find("\r\n\r\n") + 4));
}

void	phpCgi::methodPost(
	httpResponse& response,
	const httpRequest& request,
	const confLocation* location,
	const std::string& filepath) {

	int pipeFd[2];
	if (pipe(pipeFd) < 0)
		std::cout << "pipe: " << strerror(errno) << std::endl;

	int pid = fork();
	if (pid == 0) {
		close(pipeFd[0]);

		int pipeIn[2];
		pipe(pipeIn);
		dup2(pipeIn[0], 0);
		write(pipeIn[1], request.get_body().c_str(), request.get_body().size());

		char* args[] = {
			const_cast<char *>(location->_cgi_path[0].c_str()),
			const_cast<char *>(filepath.c_str()),
			NULL};

		dup2(pipeFd[1], 1);

		int err = execve(const_cast<char*>(location->_cgi_path[0].c_str()), args, _envArray);
		close(pipeFd[1]);
		exit(err);
	}

	close(pipeFd[1]);

	char buffer[BUFSIZ];
	std::string body;

	int ret = 0;
	while ((ret = read(pipeFd[0], buffer, BUFSIZ - 1)) > 0) {
		buffer[ret] = '\0';
		body += buffer;
		if (headersAreOut(body))
			body = cutHeaders(body, response);
	}

	response.set_body(body);
	response.set_content_length();
	close(pipeFd[0]);
} // methodPost

void	phpCgi::methodGet(
	httpResponse& response,
	const confLocation* location,
	const std::string& filepath) {

	int pipeFd[2];
	if (pipe(pipeFd) < 0)
		std::cout << "pipe: " << strerror(errno) << std::endl;

	int pid = fork();
	if (pid == 0) {
		close(pipeFd[0]);

		char* args[] = {
			const_cast<char *>(location->_cgi_path[0].c_str()),
			const_cast<char *>(filepath.c_str()),
			NULL};

		dup2(pipeFd[1], 1);

		int err = execve(const_cast<char*>(location->_cgi_path[0].c_str()), args, _envArray);
		close(pipeFd[1]);
		exit(err);
	}

	close(pipeFd[1]);

	char buffer[BUFSIZ];
	std::string body;

	int ret = 0;
	while ((ret = read(pipeFd[0], buffer, BUFSIZ - 1)) > 0) {
		buffer[ret] = '\0';
		body += buffer;
		if (headersAreOut(body))
			body = cutHeaders(body, response);
	}

	response.set_body(body);
	response.set_content_length();
	close(pipeFd[0]);
} // methodGet


phpCgi::phpCgi(
	const std::string& method,
	const Configuration::server* server,
	const Configuration::location* location,
	const Http::Request& request,
	httpResponse& response,
	const std::string& filepath)
{
	// if no executable or bad executable -> 502. Bad Gateway Interface

	struct stat st;
	if (stat(location->_cgi_path[0].c_str(), &st) < 0) {
		// Error 502 or 404 for not found cgi?
		pull502(response);
	}
	if (st.st_mode & S_IXUSR) {
		setEnvMap(method, server, location, request, response, filepath);
		setEnvArray();

		if (method == "GET")
			methodGet(response, location, filepath);
		else if (method == "POST")
			methodPost(response, request, location, filepath);
		else
			std::cout << "DEFAULT" << std::endl;
	}
	else // Cgi isn't an exec
		pull502(response);
} // phpCgi

void	phpCgi::pull502(httpResponse& response) {
	std::string	ret;

	/* If no file specified for 502 error */
	ret = "<!DOCTYPE html>\n";
	ret += "<html>\n";
	ret += "<body>\n";
	ret += "<h1>502: Bad Gateway Interface</h1>\n";
	ret += "</body>\n";
	ret += "</html>\n";

	response.set_status_code("502");
	response.set_body(ret);
	response.set_content_length();
	response.append_header("content_type", "text/html");
}

}; // namespace Webserv
