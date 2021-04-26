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
        response.fill_with_error(request.get_err_status_code(), server);
        return response;
    }
    const Configuration::location* location = Routing::select_location(server, request.get_uri());
    if (location == NULL) {
        response.fill_with_error("404", server);
        return response;
    }

    if (false) { // if location requires authorization
        std::list<std::string> valid_credentials = { "someuser:somepassword", "user:password" };
        if (!request.has_header("Authorization") || !Http::credentials_are_valid(request.get_header_values("Authorization"), valid_credentials)) {
            response.fill_with_error("401", server);
            response.append_header("WWW-Authenticate", "Basic");
            return response;
        }
    }

    const std::string& filepath = Routing::get_filepath(location, request.get_method(), request.get_uri());
    const std::string& method = request.get_method();
    //Log::out("Methods: ", "filepath is " + filepath);
    if (!method_is_allowed(location, method)) {
        response.fill_with_error("405", server);
    } else if (method == "GET") {
        get(response, filepath, location, server);
    } else if (method == "HEAD") {
        get(response, filepath, location, server);
        response.unset_body();
    } else if (method == "POST") {
    } else if (method == "PUT") {
        if (!request.has_header("Content-Range"))
            put(response, filepath, location->_upload_enable, request.get_body(), server);
        else
            response.fill_with_error("400", server);
    } else if (method == "DELETE") {
        delete_method(response, filepath);
    } else if (method == "OPTIONS") {
        options(response, location);
    }
    return response;
}

void put (Http::Response& response, const std::string& filepath, bool upload_enabled, const std::string& content, const Configuration::server* server)
{
    int fd;
    response.set_status_code("200");
    if (upload_enabled) {
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1)
            response.set_status_code("201");
        if (fd == -1 && (fd = open(filepath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1) {
            response.fill_with_error("403", server);
            return ;
        }
    } else {
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1) {
            response.fill_with_error("404", server);
            return ;
        }
    }
    write(fd, content.c_str(), content.size());
}

void get (Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server)
{
    int fd;
    response.set_status_code("200");
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            if (stats.st_mode & S_IFDIR) {
                if (!location->_index.empty()) {
                    std::string s;
                    if (Utils::fill_with_file_content(s, filepath + "/" + location->_index) == -1)
                        response.fill_with_error("404", server);
                    else
                        response.set_body(s);
                } else if (location->_autoindex) {
                    response.set_body(Utils::get_directory_listing(filepath));
                } else {
                    response.fill_with_error("403", server);
                }
            } else {
                char buf[stats.st_size];
                if (read(fd, &buf, stats.st_size) == stats.st_size)
                    response.set_body(std::string(buf, stats.st_size));
                else
                    response.fill_with_error("403", server);
            }
        } else {
            response.fill_with_error("500", server);
        }
        close(fd);
    } else {
        response.fill_with_error("404", server);
    }
}

void options(Http::Response &response, const Configuration::location *location)
{
    response.set_status_code("200");
    for (std::vector<std::string>::const_iterator it = location->_method.begin(); it != location->_method.end(); ++it)
        response.append_header("Allow", *it);
}

void delete_method(Http::Response &response, const std::string &filepath)
{
    response.set_status_code("200");
    std::cout << "del: " << filepath << std::endl;
    if (remove(filepath.c_str()))
        response.set_status_code("204");
}

};
};
