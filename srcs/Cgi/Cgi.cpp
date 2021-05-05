#include "Cgi.hpp"

namespace Webserv {

Cgi::Cgi() {};

Cgi::~Cgi() {
        for (int i = 0; _envArray[i]; ++i)
                delete[] _envArray[i];
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

/*
**      root location in conf should be an absolute path
**          else there could be trouble with the use of getcwd function here
*/
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

/*
**  When no data for meta-variables, the meta shouldn't be set to "" (unbless CGI spec specifiy it)
**  because isset will detect it in php's script
*/

void    Cgi::setEnvMap(
    const std::string& method,
    const confServer* server,
    const confLocation* location,
    const httpRequest& request,
    httpResponse& response,
    const std::string& filepath)
{
    const Http::HeaderMap header = request.get_headers();

    /*for (auto const &pair: header) {
        std::cout << "[" << pair.first << ":" << pair.second << "]" << std::endl;
    }
    std::cout << std::endl;*/
    //std::cout << "filepath: " << filepath << std::endl;

    /*  AUTH_TYPE (Not empty if isn't set) To check */
    if (request.has_header("authorization"))
        _envVar["AUTH_TYPE"] = request.get_header_values("authorization");

    /*  CONTENT_LENGTH  */
    if (request.has_header("content-length"))
        _envVar["CONTENT_LENGTH"] = request.get_header_values("content-length");
    else if (!request.get_body().empty()) {
        _envVar["CONTENT_LENGTH"] = ft_itos(request.get_body().length());
    }

    /*  CONTENT_TYPE    */
    if (request.has_header("content-type"))
        _envVar["CONTENT_TYPE"] = request.get_header_values("content-type");

    /*  GATEWAY_INTERFACE   */
    _envVar["GATEWAY_INTERFACE"] = "CGI/1.1";

    _envVar["PATH_INFO"] = request.get_uri();

    _envVar["PATH_TRANSLATED"] = request.get_uri();

    /*  QUERY_STRING everything after '?'
    **  contrary to what is said in the comment above the function, if there is no data,
    **  the QUERY_STRING variable MUST be defined as an empty string "". cf. CGI/1.1 specs
    */
    _envVar["QUERY_STRING"] = getQueryString(filepath);

    /*  REMOTE_ADDR (IP address from client asking the page) */
    // TMP -> should be connect in the future
    _envVar["REMOTE_ADDR"] = "127.0.0.1";

    /*  REMOTE_IDENT, probably false    */
    if (request.has_header("host"))
        _envVar["REMOTE_IDENT"] = request.get_header_values("host");

    /*  REMOTE_USER (if authorization is set)   */
    if (request.has_header("authorization"))
        std::cout << "auth USER" << std::endl;

    /*  REQUEST_METHOD  */
    _envVar["REQUEST_METHOD"] = request.get_method();

    /*  REQUEST_URI */
    _envVar["REQUEST_URI"] = request.get_uri();

    /*  SCRIPT_NAME */
    _envVar["SCRIPT_NAME"] = getScriptName(filepath, location);

    /*  SCRIPT_FILENAME (needed to exec cgi)    */
    _envVar["SCRIPT_FILENAME"] = filepath;

    /*  SERVER_NAME */
    _envVar["SERVER_NAME"] = server->_server_name;

    /*  SERVER_PORT */
    _envVar["SERVER_PORT"] = ft_itos(server->_listen.first);

    /*  SERVER_PROTOCOL */
    _envVar["SERVER_PROTOCOL"] = "HTTP/1.1";

    /*  SERVER_SOFTWARE */
    _envVar["SERVER_SOFTWARE"] = "Webserv/1.1";

    /*  REDIRECT_STATUS (Needed to tell cgi that everything was handled good by the server) */
    _envVar["REDIRECT_STATUS"] = "200";

    /*  SERVER_ADDR */
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
    body.reserve(st.st_size);

    char buffer[BUFSIZ];
    int ret = 0;
    while ((ret = read(fd, buffer, BUFSIZ)) > 0)
        body.append(buffer, ret);

    body = cutHeaders(body, response);

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
        std::cout << "ERROR FILE" << std::endl;

    int pipeInput[2];
    if (pipe(pipeInput) < 0)
        std::cout << "pipe: " << strerror(errno) << std::endl;


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
    const std::string& filepath)
{
    struct stat st;
    if (stat(location->_cgi_path[0].c_str(), &st) < 0)
        pullErrorPageCgi(response, 502, "Bad Gateway");
    if (st.st_mode & S_IXUSR) {
        setEnvMap(method, server, location, request, response, filepath);
        setEnvArray();

        if (method == "GET")
            methodGet(response, location, filepath);
        else if (method == "POST")
            methodPost(response, request, location, filepath);
    }
    else // Cgi isn't an exec
        pullErrorPageCgi(response, 502, "Bad Gateway");
} // Cgi

void    Cgi::pullErrorPageCgi(httpResponse& response, int code, const std::string& msg) {
    std::string ret;

    ret = "<!DOCTYPE html>\n";
    ret += "<html>\n";
    ret += "<body>\n";
    ret +=  "<h1>" + ft_itos(code) + ": " + msg + "</h1>\n";
    ret += "</body>\n";
    ret += "</html>\n";

    response.set_status_code(ft_itos(code));
    response.set_body(ret);
    response.set_content_length();
    response.append_header("content_type", "text/html");
} // Cgi

}; // namespace Webserv
