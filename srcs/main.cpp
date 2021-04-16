#include "Server.hpp"

// Simple Server

int main(int ac, char *av[])
{
    if (ac != 2){
        std::cout << "use ./webserv <file.conf>." << std::endl;
        return 0;
    }
    Webserv::Configuration config(av[1]);
    if (!config.get_nb_server())
        return 1;
    config.print_configuration();
    Webserv::Server server(*config.get_server(0));
        server.start();
    return 0;
}

// Multiple Server

// int main(int ac, char *av[], char *ev[])
// {
//     if (ac != 2){
//         std::cout << "use ./webserv <file.conf>." << std::endl;
//         return 0;
//     }
//     Webserv::Configuration config(av[1]);
//     if (!config.get_nb_server())
//         return 1;
//     config.print_configuration();
//     pid_t pid[config.get_nb_server()];
//     for (size_t i = 0; i < config.get_nb_server(); ++i){
//         pid[i] = fork();
//         if (pid[i] == 0){
//             Webserv::Server server(*config.get_server(i));
//             server.start();
//         }
//     }
//     for (int i = 0; i < config.get_nb_server(); ++i)
//         waitpid(pid[i], NULL, 0);
//     return 0;
// }
