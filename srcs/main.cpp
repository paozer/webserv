#include "Server_management.hpp"

// Simple Server

int main(int ac, char *av[])
{
    if (ac != 2){
        std::cout << "use ./webserv <file.conf>." << std::endl;
        return 1;
    }
    Webserv::Configuration config;
    config.parse(av[1]);
    Webserv::Server server(config);
    server.start();
    return 0;
}
