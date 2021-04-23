#ifndef ROUTING_HPP
# define ROUTING_HPP

#include "../Configuration/Configuration.hpp"

#include <algorithm>

namespace Webserv {
namespace Routing {

typedef std::vector<struct Configuration::server> vector_server_conf;

const Configuration::location* select_location (const Configuration::server* server, const std::string& uri);
const Configuration::server* select_server (const Configuration& config, const std::string& ip, const std::string& port, const std::string& host);

inline std::string get_filepath (const Configuration::location* location, const std::string& method, const std::string& uri)
{
    std::string filepath;
    if (method == "PUT" && !location->_upload_path.empty())
        filepath = location->_upload_path;
    else
        filepath = location->_root;
    filepath += uri.substr(location->_name.length());
    return filepath;
}

inline bool compare_locations (const struct Configuration::location* a, const struct Configuration::location* b)
{
    return a->_name.length() < b->_name.length();
}

}; // namespace Webserv
}; // namespace Routing

# endif
