#include "Methods.hpp"

namespace Webserv {
namespace Methods {

Http::Response method_handler (const Http::Request& request, const Configuration& config)
{
    Http::Response response;
    if (!request.has_header("Host")) {
        // TODO move to Request class
        response.set_status_code("400");
        return response;
    }

    std::string host = request.get_header_values("host");
    // ip and port that bind was called with i.e. related to the socket
    const Configuration::server* server = Routing::select_server(config, "127.0.0.1", "8080", host);
    const Configuration::location* location = Routing::select_location(server, request.get_uri());

    if (location == NULL) {
        response.set_status_code("404");
        return response;
    }

    const std::string& filepath = Routing::get_filepath(location->_name, location->_root, request.get_uri());
    const std::string& method = request.get_method();
    if (!method_is_allowed(location, method)) {
        response.set_status_code("405");
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
            response.set_status_code("400");
    } else if (method == "DELETE") {
        // delete uri
    } else if (method == "CONNECT") {
        // connect to the server identified by uri
    } else if (method == "OPTIONS") {
        // return available commnunications options
    } else if (method == "TRACE") {
        // return what was received
    }
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
        // if file does not exist set status_code to 201
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) == -1) {
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
                    response.set_body(get_file_content(location->_root + location->_index));
                } else if (location->_autoindex) {
                    response.set_body(get_directory_listing(filepath));
                } else {
                    response.set_status_code("403");
                }
                response.set_content_length();
            } else {
                char buf[stats.st_size];
                if (read(fd, &buf, stats.st_size) == stats.st_size) {
                    response.set_status_code("200");
                    response.set_body(std::string(buf, stats.st_size));
                    response.set_content_length();
                } else {
                    response.set_status_code("403");
                }
            }
        } else {
            response.set_status_code("500");
        }
        close(fd);
    } else {
        response.set_status_code("403");
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
