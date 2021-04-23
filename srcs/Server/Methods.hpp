#ifndef METHODS_HPP
# define METHODS_HPP

#include "Routing.hpp"
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Utils.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <algorithm>

namespace Webserv {
namespace Methods {

Http::Response method_handler (const Http::Request& request, const Configuration& config);
void get (Http::Response& response, const std::string& filepath, const Configuration::location* location);
void put (Http::Response& response, const std::string& filepath, bool upload_enabled, const std::string& content);

void fill_error_response (Http::Response& response, const std::string& status_code, const Configuration::server* server);
std::string get_directory_listing (const std::string& path);
std::string get_file_content (const std::string& filepath);
int fill_with_file_content (std::string& s, const std::string& filepath);

inline bool method_is_allowed (const Configuration::location* location, const std::string& method)
{
    return std::find(location->_method.begin(), location->_method.end(), method) != location->_method.end();
}

};
};

#endif
