#include "Methods.hpp"
#include "../Cgi/phpCgi.hpp"

namespace Webserv {
namespace Methods {

//response.append_header("Content-Location", ""); // TODO set to alternative uri for requested ressource

// TODO if uri maps to different ressources parse their
// charset/language and return the most appropriate
//response.append_header("Accept-Charsets", "");
//response.append_header("Accept-Language", "");
//response.append_header("Content-Language", "");

//AUTHORIZATION used in authentication
//ALLOW used in OPTIONS and 405 error
//CONTENT-LENGTH used for all responses
//CONTENT-TYPE used in GET/POST TODO POST
//HOST used in request routing
//TRANSFER-ENCODING used when receiving/sending chunked body
//LAST-MODIFIED used in GET/PUT when a ressource was created TODO POST
//LOCATION used in PUT/POST when a ressource was created TODO POST
//REFERER this is not useful
//RETRY-AFTER used when bouncing clients
//USER-AGENT this is not useful
//WWW-AUTHENTICATE used for unauthenticated requests

Http::Response method_handler (const Http::Request& request, const Configuration& config, int fd)
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(fd, (struct sockaddr *)&sin, &len);
    const Configuration::server* server = Routing::select_server(config, Http::inet_ntoa(sin.sin_addr.s_addr), Utils::itoa(Http::mhtons(sin.sin_port)), request.get_header_values("host"));

    Http::Response response = Http::Response::create_standard_response();
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
    if (request.get_body().length() > static_cast<size_t>(location->_client_max_body_size)) {
        response.fill_with_error("413", server);
    } else if (!method_is_allowed(location, method)) {
        response.fill_with_error("405", server);
    } else if (method == "GET") {
        get(response, filepath, location, server);
    } else if (method == "HEAD") {
        get(response, filepath, location, server);
        response.set_content_length();
        response.unset_body();
    } else if (method == "POST") {
        if (request.get_uri().rfind(location->_cgi_extension) == request.get_uri().size() - location->_cgi_extension.size()) {
            CgiHandler CgiHandler(request, location);
            CgiHandler.executeCgi(location->_cgi_path[0], response);
            // Log::out("methods", "cgi called");
        } else if (!request.has_header("Content-Range")) {
            response.set_status_code("204");
            response.set_body("");
            // put(response, filepath, location->_upload_enable, request.get_body());
        } else
            response.fill_with_error("400", server);
    } else if (method == "PUT") {
        put(request, response, filepath, location, server);
    } else if (method == "DELETE") {
        mdelete(response, filepath, server);
    } else if (method == "OPTIONS") {
        options(response, location);
    }
    return response;
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
                    if (Files::fill_with_file_content(s, filepath + "/" + location->_index) == -1)
                        response.fill_with_error("404", server);
                    else
                        response.set_body(s);
                } else if (location->_autoindex) {
                    response.set_body(Files::get_directory_listing(filepath));
                    response.append_header("Content-Type", "text/html");
                } else {
                    response.fill_with_error("403", server);
                }
            } else {
                char buf[stats.st_size];
                if (read(fd, &buf, stats.st_size) == stats.st_size) {
                    response.set_body(std::string(buf, stats.st_size));
                    response.append_header("Content-Type", get_media_type(filepath));
                } else {
                    response.fill_with_error("403", server);
                }
            }
            if (response.get_status_code() != "403" && response.get_status_code() != "404") {
                response.append_header("Last-Modified", Time::get_http_formatted_date(stats.st_mtime));
            }
        } else {
            response.fill_with_error("500", server);
        }
        close(fd);
    } else {
        response.fill_with_error("404", server);
    }
}


void put (const Http::Request& request, Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server)
{
    if (request.has_header("Content-Range")) {
        response.fill_with_error("400", server);
        return ;
    }
    int fd;
    if (location->_upload_enable) {
        if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1) {
            response.set_status_code("201");
        }
        if (fd == -1 && (fd = open(filepath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1) {
            response.fill_with_error("403", server);
            return ;
        }
        if (response.get_status_code() == "201") { // if ressource was created
            response.append_header("Location", request.get_uri()); // TODO is this correct ?
        } else { // else if it existed previously and it could be opened
            struct stat st;
            if (fstat(fd, &st) == 0)
                response.append_header("Last-Modified", Time::get_http_formatted_date(st.st_mtime));
        }
    } else if ((fd = open(filepath.c_str(), O_WRONLY | O_TRUNC)) == -1) {
        response.fill_with_error("404", server);
        return ;
    }
    write(fd, request.get_body().c_str(), request.get_body().size());
    close(fd);
}

void options (Http::Response &response, const Configuration::location *location)
{
    for (std::vector<std::string>::const_iterator it = location->_method.begin(); it != location->_method.end(); ++it)
        response.append_header("Allow", *it);
}

void mdelete (Http::Response &response, const std::string &filepath, const Configuration::server* server)
{
    int fd;
    struct stat st;
    if (stat(filepath.c_str(), &st) == 0 && (fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        if (st.st_mode & S_IFDIR)
            rmdir(filepath.c_str());
        else
            unlink(filepath.c_str());
        close(fd);
    } else {
        response.fill_with_error("404", server);
    }
}

};
};
