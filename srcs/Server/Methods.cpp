#include "Methods.hpp"

namespace Webserv {
namespace Methods {

Http::Response create_standard_response (void)
{
    Http::Response response;
    response.set_status_code("200");
    response.append_header("Date", Time::get_date_header_format());
    response.append_header("Server", "webserv/1.0");
    //response.append_header("Retry-After", ""); // TODO set when bouncing clients
    //response.append_header("Last-Modified", ""); // TODO set for GET / POST / PUT same format as Date
    //response.append_header("Content-Type", ""); // TODO set for GET using mime types
    //response.append_header("Location", ""); // TODO set to newly created file when returning 201 from POST/PUT
    //response.append_header("Content-Location", ""); // TODO set to alternative uri for requested ressource

    // TODO if uri maps to different ressources parse their
    // charset/language and return the most appropriate
    //response.append_header("Accept-Charsets", "");
    //response.append_header("Accept-Language", "");
    //response.append_header("Content-Language", "");

    //response.append_header("Allow", ""); // used in OPTIONS method and 405 error
    //response.append_header("Content-Length", ""); // used for all responses
    //response.append_header("Host", ""); // used in request routing
    //response.append_header("Authorization", ""); // used in authentication
    //response.append_header("WWW-Authenticate", ""); // used for unauthenticated requests
    //response.append_header("Transfer-Encoding", ""); // used when receiving/sending chunked body
    //response.append_header("Referer", ""); // this is not useful
    //response.append_header("User-Agent", ""); // this is not useful
    return response;
}

Http::Response method_handler (const Http::Request& request, const Configuration& config, int fd)
{
    std::string host = request.has_header("host") ? request.get_header_values("host") : "";
    // ip and port that bind was called with i.e. related to the socket

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(fd, (struct sockaddr *)&sin, &len);

    // ip and port that bind was called with i.e. related to the socket
    const Configuration::server* server = Routing::select_server(config, Utils::inet_ntoa(sin.sin_addr.s_addr), Utils::itoa(Utils::m_htons(sin.sin_port)), host);
    Http::Response response = create_standard_response();
    if (request.get_state() == Http::Error) {
        response.fill_with_error(request.get_err_status_code(), server);
        return response;
    }
    const Configuration::location* location = Routing::select_location(server, request.get_uri());
    if (location == NULL) {
        response.fill_with_error("404", server);
        return response;
    }

    if (location->auth != "off") {
        if (!request.has_header("Authorization") ||
                !Http::credentials_are_valid(request.get_header_values("Authorization"), location->auth_credentials)) {
            response.fill_with_error("401", server);
            response.append_header("WWW-Authenticate", "Basic");
            return response;
        }
    }

    const std::string& filepath = Routing::get_filepath(location, request.get_method(), request.get_uri());
    const std::string& method = request.get_method();
    if (request.get_body().length() > location->_client_max_body_size) {
        response.fill_with_error("413", server);
    } else if (!method_is_allowed(location, method)) {
        response.fill_with_error("405", server);
    } else if (method == "GET") {
        get(response, filepath, location, server);
    } else if (method == "HEAD") {
        get(response, filepath, location, server);
        response.unset_body();

    } else if (method == "POST") {
        if (request.get_uri().rfind(location->_cgi_extension) == request.get_uri().size() - location->_cgi_extension.size()) {
            // Log::out("methods", "cgi called");
            CgiHandler cgi(request, location);
            cgi.executeCgi(location->_cgi_path[0], response);
        } else if (!request.has_header("Content-Range")) {
            response.set_status_code("204");
            response.set_body("");
            // put(response, filepath, location->_upload_enable, request.get_body());
        } else
            response.fill_with_error("400", server);


    } else if (method == "PUT") {
        // Log::out("filepath is ", filepath);
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
    close(fd);
}

void get (Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server)
{
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            if (stats.st_mode & S_IFDIR) {
                if (!location->_index.empty()) {
                    std::string s;
                    if (Files::fill_with_file_content(s, filepath + "/" + location->_index) == -1) {
                        response.fill_with_error("404", server);
                    } else {
                        response.set_body(s);
                        // add last-mod
                    }
                } else if (location->_autoindex) {
                    response.set_body(Files::get_directory_listing(filepath));
                    // add last-mod
                } else {
                    response.fill_with_error("403", server);
                }
            } else {
                char buf[stats.st_size];
                if (read(fd, &buf, stats.st_size) == stats.st_size) {
                    response.set_body(std::string(buf, stats.st_size));
                    // add last-mod
                } else {
                    response.fill_with_error("403", server);
                }
            }
        } else {
            response.fill_with_error("500", server);
        }
        close(fd);
    } else {
        response.fill_with_error("404", server);
    }
}

void options (Http::Response &response, const Configuration::location *location)
{
    for (std::vector<std::string>::const_iterator it = location->_method.begin(); it != location->_method.end(); ++it)
        response.append_header("Allow", *it);
}

// TODO error status code
void delete_method (Http::Response &response, const std::string &filepath)
{
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            if (stats.st_mode & S_IFDIR) {
                rmdir(filepath.c_str());
            } else {
                unlink(filepath.c_str());
            }
        }
    }
}

};
};
