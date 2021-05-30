#include "Cgi.hpp"

namespace Webserv {

Cgi::~Cgi() {
    if (_envVar.size() != 0) {
        for (int i = 0; _envArray[i]; ++i)
            delete[] _envArray[i];
    }
    delete[] _envArray;
}

std::string Cgi::getScriptName(const std::string& filepath, const confLocation* location) {
    std::size_t start = filepath.find(location->_root, 0);
    start += location->_root.length();

    std::string ret = filepath.substr(start, filepath.length());
    std::size_t end = ret.find(location->_cgi_extension, 0);
    ret = ret.substr(0, end + 4);
    ret.insert(0, "/");
    return (ret);
} // getScriptName

std::string Cgi::getPathInfo(const std::string& filepath, const confLocation* location) {
    std::string ret;

    std::size_t start = filepath.find(location->_cgi_extension, 0);
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
} // getPathInfo

std::string Cgi::getFullPath(const std::string& filepath, const confLocation* location) {
    std::string ret;
    // Get script name and remove the first '/'
    std::string name = getScriptName(filepath, location);
    name = name.substr(1, name.size());

    char *buf = new char[500];
    ret = getcwd(buf, 300);
    if (location->_root[0] == '.')
        ret += location->_root.substr(1, location->_root.length());
    else
        ret += location->_root;
    ret += name;
    return (ret);
} // getFullPath

std::string Cgi::getRequestUri(const std::string& filepath, const confLocation* location) {
    std::string ret = filepath;
    std::size_t end = ret.find(location->_cgi_extension, 0);
    ret = ret.substr(0, end + 4);
    ret.erase(0, location->_root.length());
    ret.insert(0, "/");
    return (ret);
} // getRequestUri

std::string Cgi::getQueryString(const std::string& filepath) {
    std::size_t start = filepath.find("?", 0);

    if (start != std::string::npos) {
        start++;
        std::string ret(filepath.begin() + start, filepath.end());
        return (ret);
    }
    else
        return ("");
} // getQueryString

char    *Cgi::assembleMetaVar(const std::string& key, const std::string& value) {
    std::string res = key + "=" + value;
    char *ret = new char[res.size() + 1];
    strcpy(ret, res.c_str());

    return (ret);
} // assembleMetaVar

int     Cgi::getLenNumber(int n) {
    int ret = 0;

    while (n > 0) {
        n /= 10;
        ret++;
    }
    return (ret);
} // getLenNumber

std::string Cgi::ft_itos(const int& ipt) {
    int     n = ipt;
    int     nLen = getLenNumber(n);
    std::string ret(nLen, '\0');

    nLen--;
    while (n > 0) {
        ret[nLen] = (n % 10) + 48;
        nLen--;
        n /= 10;
    }
    return (ret);
} // ft_itos

void    Cgi::setEnvArray() {
    std::size_t len = _envVar.size();
    _envArray = new char*[len + 1];

    int count = 0;
    envMap::iterator it = _envVar.begin();
    for (; it != _envVar.end(); it++) {
        _envArray[count++] = assembleMetaVar(it->first, it->second);
    }
    _envArray[count] = NULL;
} // setEnvArray

std::string Cgi::formatNameKey(const std::string& name) {
    std::string ret = "HTTP_" + name;
    std::size_t length = ret.length();

    for (std::size_t i = 0; i < length; ++i) {
        if (ret[i] == '-')
            ret[i] = '_';
        else
            ret[i] = toupper(ret[i]);
    }
    return (ret);
} // formatNameKey

void    Cgi::setEnvMap(
    const confServer* server,
    const confLocation* location,
    const httpRequest& request,
    const std::string& filepath,
    const std::string& client_address)
{
    const Http::HeaderMap header = request.get_headers();

    if (request.has_header("content-length"))
        _envVar["CONTENT_LENGTH"] = request.get_header_values("content-length");
    else if (!request.get_body().empty()) {
        _envVar["CONTENT_LENGTH"] = ft_itos(request.get_body().length());
    }

    if (request.has_header("content-type"))
        _envVar["CONTENT_TYPE"] = request.get_header_values("content-type");

    _envVar["GATEWAY_INTERFACE"] = "CGI/1.1";
    _envVar["PATH_INFO"] = request.get_uri();
    _envVar["PATH_TRANSLATED"] = request.get_uri();
    _envVar["QUERY_STRING"] = getQueryString(filepath);
    _envVar["REMOTE_ADDR"] = client_address;
    _envVar["REMOTE_IDENT"] = request.get_header_values("host");

    if (request.has_header("authorization")) {
        _envVar["AUTH_TYPE"] = "Basic";
        std::string id = request.get_header_values("authorization");
        id.erase(0, id.find(" ") + 1);
        id = Http::base64_decode(id);
        _envVar["REMOTE_USER"] = id.substr(0, id.find(":"));
    }

    _envVar["REQUEST_METHOD"] = request.get_method();
    _envVar["REQUEST_URI"] = request.get_uri();
    _envVar["SCRIPT_NAME"] = getScriptName(filepath, location);
    _envVar["SCRIPT_FILENAME"] = filepath;
    _envVar["SERVER_NAME"] = server->_server_name;
    _envVar["SERVER_PORT"] = ft_itos(server->_listen.first);
    _envVar["SERVER_PROTOCOL"] = "HTTP/1.1";
    _envVar["SERVER_SOFTWARE"] = "Webserv/1.1";
    _envVar["REDIRECT_STATUS"] = "200";
    _envVar["SERVER_ADDR"] = server->_listen.second;

    // Add all custom headers (starts with "X-")
    const Http::HeaderMap tmpMap = request.get_headers();
    Http::HeaderMap::const_iterator start = tmpMap.lower_bound("X-");
    Http::HeaderMap::const_iterator end = tmpMap.upper_bound("X-");
    if (start != tmpMap.end()) {
        ++end;
        while (start != end) {
            _envVar[formatNameKey(start->first)] = start->second;
            start++;
        }
    }

} // setEnvMap

std::string Cgi::pullCode(std::string& value) {
    std::string ret;
    std::size_t start;

    start = value.find_first_of("0123456789");
    for (std::size_t i = start; value[i] >= '0' && value[i] <= '9'; i++)
            ret += value[i];
    return (ret);
} // pullCode

std::string Cgi::cutHeaders(const std::string& body, httpResponse& response) {
    bool        status = false;
    std::size_t start = 0;
    std::string name;
    std::string value;

    if (body.find("\r\n\r\n") == std::string::npos)
        return (body);
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
    if (!status) {
        response.set_status_code("200");
    }
    return(body.substr(body.find("\r\n\r\n") + 4));
} // cutHeaders

std::string Cgi::generateRandomName() {
    srand(time(0));

    std::string name = "tmp_" + ft_itos(rand() % 100000) + ft_itos(rand() % 10);
    struct stat st;
    while (stat(name.c_str(), &st) >= 0)
        name = "tmp_" + ft_itos(rand() % 100000) + ft_itos(rand() % 10);
    return (name);
} // generateRandomName

void    Cgi::processingPostOutput(int fd, std::string& name, httpResponse& response) {
    lseek(fd, 0, SEEK_SET);
    struct stat st;
    fstat(fd, &st);
    std::string body;
    body.reserve(st.st_size + 500);

    char buffer[1024];
    int ret = 1024;
    while (ret == 1024 && body.find("\r\n\r\n") == std::string::npos) {
        ret = read(fd, buffer, 1024);
        body.append(buffer, ret);
    }
    body = cutHeaders(body, response);
    char *rbuf = new char[st.st_size];
    ret = read(fd, rbuf, st.st_size);
    body.append(rbuf, ret);
    delete[] rbuf;
    close(fd);
    if (unlink(name.c_str()) < 0)
        Log::out(name, strerror(errno));
    response.set_body(body);
    response.set_content_length();

} // processingPostOutput

void    Cgi::methodPost(
    httpResponse& response,
    const httpRequest& request,
    const confLocation* location,
    const std::string& filepath) {

    int tmpFd;
    std::string name = generateRandomName();
    if ((tmpFd = open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
        Log::out(location->_cgi_path[0], strerror(errno));

    int pipeInput[2];
    if (pipe(pipeInput) < 0)
        Log::out(location->_cgi_path[0], strerror(errno));


    int pid = fork();
    if (pid < 0) {
        close(pipeInput[0]);
        close(pipeInput[1]);
    }
    else if (pid == 0) {

        close(pipeInput[1]);

        char* args[] = {
            const_cast<char *>(location->_cgi_path[0].c_str()),
            const_cast<char *>(filepath.c_str()),
            NULL};

        dup2(pipeInput[0], 0);
        dup2(tmpFd, 1);

        execve(const_cast<char*>(location->_cgi_path[0].c_str()), args, _envArray);
        exit(-1);
    }
    else {
        close(pipeInput[0]);
        write(pipeInput[1], request.get_body().c_str(), request.get_body().length());
        close(pipeInput[1]);

        waitpid(-1, NULL, 0);
        processingPostOutput(tmpFd, name, response);
    }
} // methodPost

void    Cgi::methodGet(
    httpResponse& response,
    const confLocation* location,
    const std::string& filepath) {

    int pipeFd[2];
    if (pipe(pipeFd) < 0)
        Log::out(location->_cgi_path[0], strerror(errno));

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
    waitpid(-1, NULL, 0);

    char buffer[BUFSIZ];
    std::string body;

    int ret = 0;
    while ((ret = read(pipeFd[0], buffer, BUFSIZ)) > 0)
        body.append(buffer, ret);

    body = cutHeaders(body, response);

    response.set_body(body);
    response.set_content_length();
    close(pipeFd[0]);
} // methodGet


Cgi::Cgi(
    const std::string& method,
    const Configuration::server* server,
    const Configuration::location* location,
    const Http::Request& request,
    httpResponse& response,
    const std::string& filepath,
    const std::string& client_address)
{
    _envArray = NULL;
    
    struct stat st;
    if (stat(location->_cgi_path[0].c_str(), &st) < 0)
        response.fill_with_error("502", server);
    else if (st.st_mode & S_IXUSR) {
        setEnvMap(server, location, request, filepath, client_address);
        setEnvArray();

        if (method == "GET")
            methodGet(response, location, filepath);
        else if (method == "POST")
            methodPost(response, request, location, filepath);
    }
    else
        response.fill_with_error("502", server);
} // Cgi

}; // namespace Webserv
