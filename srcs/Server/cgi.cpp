#include "cgi.hpp"

namespace Webserv {

CgiHandler::CgiHandler(const Http::Request &request, const Configuration::location *config):
_body(request.get_body())
{
    (void)config;
    this->_initEnv(request, *config);
}

void        CgiHandler::_initEnv(const Http::Request &request, const Configuration::location &config) {
    Http::HeaderMap headers = request.get_headers();
    if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
        this->_env["AUTH_TYPE"] = headers["Authorization"];

    this->_env["REDIRECT_STATUS"] = "200";
    this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->_env["SCRIPT_NAME"] = "YoupiBanane/directory/youpi.bla";
    this->_env["SCRIPT_FILENAME"] = "YoupiBanane/directory/youpi.bla";
    this->_env["REQUEST_METHOD"] = request.get_method();
    this->_env["CONTENT_LENGTH"] = Utils::itoa(request.get_body().length());
    this->_env["CONTENT_TYPE"] = headers["Content-Type"];
    this->_env["PATH_INFO"] = request.get_uri();
    this->_env["PATH_TRANSLATED"] = request.get_uri();
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
}

char                    **CgiHandler::_getEnvAsCstrArray() const {
    char    **env = new char*[this->_env.size() + 1];
    int j = 0;
    for (std::map<std::string, std::string>::const_iterator i = this->_env.begin(); i != this->_env.end(); i++) {
        std::string element = i->first + "=" + i->second;
        env[j] = new char[element.size() + 1];
        env[j] = strcpy(env[j], (const char*)element.c_str());
        j++;
    }
    env[j] = NULL;
    return env;
}

std::string rand_string()
{
    char choice[7] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    std::srand(time(NULL));
    std::string name;
    for (int i = 0; i < 4; ++i)
        name.push_back(choice[std::rand() % 7]);
    return name;
}

std::string get_unique_name()
{
    std::string name;
    struct stat buf;
    for (name = rand_string(); !stat(name.c_str(), &buf); name = rand_string());
    return name.c_str();
}

void construct_body(Http::Response &response, std::string &name, int fd)
{
    struct stat stat_buf;
    std::string body;
    size_t      i = 0;
    size_t      j = - 2;
    char        *buffer = new char[10000000];

    stat(name.c_str(), &stat_buf);
    body.reserve(stat_buf.st_size + 1);
    lseek(fd, 0, SEEK_SET);
    for (int ret = 1; ret > 0; body.append(buffer, ret))
        ret = read(fd, buffer, 1000000);
    close(fd);
    delete[] buffer;
    response.set_status_code("200");
    while (body.find("\r\n\r\n", i) != std::string::npos || body.find("\r\n", i) == i)
    {
        std::string str = body.substr(i, body.find("\r\n", i) - i);
        if (str.find("Status: ") == 0)
            response.set_status_code(str.substr(8, 3).c_str());
        else if (str.find("Content-type: ") == 0)
            response.append_header("Content-Type", str.substr(14, str.size()));
        i += str.size() + 2;
    }
    while (body.find("\r\n", j) == j)
        j -= 2;
    body = body.substr(i, j - i);
    response.set_body(body);
}

void        CgiHandler::executeCgi(const std::string& scriptName, Http::Response &response) {
    char        **env;
    std::string newBody;

    try {
        env = this->_getEnvAsCstrArray();
    }
    catch (std::bad_alloc &e) {
        std::cerr << e.what() << std::endl;
    }
    std::string name = get_unique_name();
    int tmp_file = open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int p[2];
    pipe(p);
    pid_t pid = fork();
    if (pid == -1) {
        close(p[0]);
        close(p[1]);
    } else if (!pid) {
        close(p[1]);
        dup2(tmp_file, 1);
        dup2(p[0], 0);
        execve(scriptName.c_str(), NULL, env);
        write(tmp_file, "Status: 500\r\n\r\n", 15);
    } else {
        close(p[0]);
        write(p[1], _body.c_str(), _body.length());
        close(p[1]);
        waitpid(-1, NULL, 0);
        construct_body(response, name, tmp_file);
    }
    for (size_t i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;
    unlink(name.c_str());
    // std::cout << response.get_status_code() << " | " << response.get_body().length() << std::endl;
}

};
