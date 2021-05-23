#ifndef METHODS_HPP
#define METHODS_HPP

#include "Routing.hpp"
#include "../Http/Authentication.hpp"
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Utils/Files.hpp"
#include "../Utils/Utils.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Cgi/Cgi.hpp"
#include "./ContentNegotiation/ContentNegotiation.hpp"

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

namespace Webserv {
namespace Methods {

Http::Response method_handler (const Http::Request& request, const Configuration& config, int cli_socket);

void get (const Http::Request& request, Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server);
void post (const Http::Request& request, Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server);
void put (const Http::Request& request, Http::Response& response, const std::string& filepath, const Configuration::location* location, const Configuration::server* server);
void options(Http::Response &response, const Configuration::location *location);
void mdelete (Http::Response &response, const std::string &filepath, const Configuration::server* server);

inline bool method_is_allowed (const Configuration::location* location, const std::string& method)
{
    return std::find(location->_method.begin(), location->_method.end(), method) != location->_method.end();
}

};
};

#endif
