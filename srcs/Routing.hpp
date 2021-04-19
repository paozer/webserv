#pragma once

#include "Config.hpp"

#include <algorithm>

namespace Webserv {
namespace Routing {

typedef std::vector<struct Configuration::server> vector_server_conf;

const Configuration::location* select_location (const Configuration::server* server, const std::string& uri);
const Configuration::server* select_server (const Configuration& config, const std::string& ip, const std::string& port, const std::string& host);

inline std::string get_filepath (const std::string& location, const std::string& root, const std::string& uri)
{
    std::string filepath = root;
    filepath += uri.substr(location.length());
    return filepath;
}

inline bool compare_locations (const struct Configuration::location* a, const struct Configuration::location* b)
{
    return a->_name.length() < b->_name.length();
}

}; // namespace Webserv
}; // namespace Routing
