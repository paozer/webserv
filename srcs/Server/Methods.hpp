#ifndef METHODS_HPP
#define METHODS_HPP

#include "Routing.hpp"
#include "ServerSocket.hpp"
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Http/Authentication.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Files.hpp"
#include "../Utils/Utils.hpp"
#include "cgi.hpp"
#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <algorithm>

namespace Webserv {
namespace Methods {

void get (Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server);
void put (Http::Response& response, const std::string& filepath, bool upload_enabled, const std::string& content, const Configuration::server* server);
Http::Response method_handler (const Http::Request& request, const Configuration& config, int fd);
void options(Http::Response &response, const Configuration::location *location);
void delete_method(Http::Response &response, const std::string &filepath);

void fill_error_response (Http::Response& response, const std::string& status_code, const Configuration::server* server);

inline bool method_is_allowed (const Configuration::location* location, const std::string& method)
{
    return std::find(location->_method.begin(), location->_method.end(), method) != location->_method.end();
}

};
};

#endif
