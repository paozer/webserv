#include "Methods.hpp"

namespace Webserv {
namespace Methods {

Http::Response method_handler (const Http::Request& request, const Configuration& config)
{
    std::string host = request.has_header("host") ? request.get_header_values("host") : "";

    // ip and port that bind was called with i.e. related to the socket
    const Configuration::server* server = Routing::select_server(config, "127.0.0.1", "8080", host);
    Http::Response response;
    if (request.get_state() == Http::Error) {
        fill_error_response(response, request.get_err_status_code(), server);
        return response;
    }
    const Configuration::location* location = Routing::select_location(server, request.get_uri());
    if (location == NULL) {
        fill_error_response(response, "404", server);
        return response;
    }

    const std::string& filepath = Routing::get_filepath(location, request.get_method(), request.get_uri());
    Log::out("Methods: ", "filepath is " + filepath);

    const std::string& method = request.get_method();
    if (!method_is_allowed(location, method)) {
        fill_error_response(response, "405", server);
    } else if (method == "GET") {
        get(response, filepath, location);
    } else if (method == "HEAD") {
        get(response, filepath, location);
        response.unset_body();
    } else if (method == "POST") {
        // execute ressource
    } else if (method == "PUT") {
        if (!request.has_header("Content-Range"))
            put(response, filepath, location->_upload_enable, request.get_body());
        else
            fill_error_response(response, "400", server);
    } else if (method == "DELETE") {
        // delete uri
    } else if (method == "CONNECT") {
        // connect to the server identified by uri
    } else if (method == "OPTIONS") {
        // return available commnunications options
    } else if (method == "TRACE") {
        // return what was received
    }
    // TODO call fill_error_response in http method implementations
    fill_error_response(response, response.get_status_code(), server);
    return response;
}

void put (Http::Response& response, const std::string& filepath, bool upload_enabled, const std::string& content)
{
    // TODO better permissions
    //  upload_enabled > we can create new files
    // !upload_enabled > we can only replace files
    int fd;
    response.set_status_code("200");
    if (upload_enabled) {
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1)
            response.set_status_code("201");
        if (fd == -1 && (fd = open(filepath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) == -1) {
            response.set_status_code("403");
            return ;
        }
    } else {
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1) {
            response.set_status_code("404");
            return ;
        }
    }
    write(fd, content.c_str(), content.size());
}

void get (Http::Response& response, const std::string& filepath, const Configuration::location* location)
{
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            if (stats.st_mode & S_IFDIR) {
                response.set_status_code("200");
                if (!location->_index.empty()) {

                    std::string s;
                    if (fill_with_file_content(s, filepath + "/" + location->_index) == -1)
                        response.set_status_code("404");
                    else
                        response.set_body(s);

                } else if (location->_autoindex) {
                    response.set_body(get_directory_listing(filepath));
                } else {
                    response.set_status_code("403");
                }
            } else {
                char buf[stats.st_size];
                if (read(fd, &buf, stats.st_size) == stats.st_size) {
                    response.set_status_code("200");
                    response.set_body(std::string(buf, stats.st_size));
                } else {
                    response.set_status_code("403");
                }
            }
        } else {
            response.set_status_code("500");
        }
        close(fd);
    } else {
        response.set_status_code("404");
    }
}

int fill_with_file_content (std::string& s, const std::string& filepath)
{
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            char buf[stats.st_size];
            if (read(fd, &buf, stats.st_size) == stats.st_size)
                s = std::string(buf, stats.st_size);
        }
        close(fd);
        return 0;
    }
    return -1;
}

void fill_error_response (Http::Response& response, const std::string& status_code, const Configuration::server* server)
{
    response.set_status_code(status_code);
    if (server != NULL) {
        std::map<int, std::string>::const_iterator it = server->_error_pages.find(Utils::atoi(status_code));
        if (it != server->_error_pages.end())
            response.set_body(get_file_content(it->second));
    }
}

std::string get_file_content (const std::string& filepath)
{
    std::string s;
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            char buf[stats.st_size];
            if (read(fd, &buf, stats.st_size) == stats.st_size) {
                s = std::string(buf, stats.st_size);
            }
        }
        close(fd);
    }
    return s;
}

std::string get_directory_listing (const std::string& path)
{
    std::string s;
    DIR * dir = opendir(path.c_str());
    for (struct dirent* dir_entry; (dir_entry = readdir(dir)) != NULL; ) {
        s += dir_entry->d_name;
        s += "\n";
    }
    closedir(dir);
    return s;
}

};
};
