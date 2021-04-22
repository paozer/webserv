#include "Routing.hpp"

namespace Webserv {
namespace Routing {

const Configuration::location* select_location (const Configuration::server* server, const std::string& uri)
{
    if (server == NULL)
        return NULL;
    std::vector<const struct Configuration::location*> selected_locations;
    for (std::vector<struct Configuration::location>::const_iterator it = server->_locations.begin(); it != server->_locations.end(); ++it) {
        if (uri.find(it->_name) == 0)
            selected_locations.push_back(&*it);
    }
    if (selected_locations.empty())
        return NULL;
    return *std::max_element(selected_locations.begin(), selected_locations.end(), compare_locations);
}

const Configuration::server* select_server (const Configuration& config, const std::string& ip, const std::string& port, const std::string& host)
{
    size_t i = host.find(":");
    std::string host_name = host.substr(0, i);
    std::string host_port = (i == std::string::npos ? "80" : host.substr(i + 1));
    const vector_server_conf& servers = config.get_servers();
    std::vector<const struct Configuration::server*> selected_servers;

    for (vector_server_conf::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->_listen.second == ip && it->_listen.first == Utils::atoi(port.c_str()))
            selected_servers.push_back(&(*it));
    }
    if (selected_servers.empty()) {
        // this should never happen
        return NULL;
    } else if (selected_servers.size() > 1) {
        for (vector_server_conf::const_iterator it = servers.begin(); it != servers.end(); ++it) {
            if (it->_server_name == host_name)
                return &*it;
        }
    }
    return &*selected_servers[0];
}

}; // namespace Webserv
}; // namespace Routing
