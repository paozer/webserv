#include "Server.hpp"

int main(int ac, char *av[], char *ev[])
{
    if (ac != 2){
        std::cout << "use ./webserv <file.conf>." << std::endl;
        return 0;
    }
    Webserv::Configuration config(av[1]);
    if (!config.get_nb_server())
        return 1;
    config.print_configuration();
    for (size_t i = 0; i < config.get_nb_server(); ++i){
        std::cout << "Launch server...\n" << std::endl;
        Webserv::Server server(*config.get_server(i));
        server.start();
    }
    std::cout << "Server shutdown...\n" << std::endl;
    return 0;
}
