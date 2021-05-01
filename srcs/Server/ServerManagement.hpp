#ifndef SERVER_MANAGEMENT
# define SERVER_MANAGEMENT

#include "ConnectionsManagement.hpp"
#include "ServerSocket.hpp"
#include "Worker.hpp"
#include "../Configuration/Parsing.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Utils.hpp"
#include "../Utils/Logger.hpp"

#include <sys/select.h>
#include <unistd.h> // close, unlink
#include <signal.h>
#include <pthread.h>

#if defined(__linux__)
#include <bits/stdc++.h>
#endif

#include <string>
#include <vector>


namespace Webserv {

static bool stop_server;

class Server
{
    public:
        Server(Configuration const &config);
        ~Server();
        void start();

    private:
        int                     _nb_worker;
        std::string             _s_buffer;

        Configuration           _configuration;
        ConnectionManagement    _connections;
        worker_config           *_workers;
        std::vector<ServerSocket>  _socket;

        struct sockaddr_in      _cli_addr;
        unsigned int            _cli_len;
        int                     _cli_sock;

        int                     total_connections;
        time_t                    start_time;

        void                    main_loop_with_workers();
        void                    main_loop_without_workers();
        void                    sockets_settings();
        int                     get_available_worker();
        void                    new_worker(std::string const &socket_details);
        void                    refused_connection(std::string const &socket_details);
};

void signal_handler(int sig);

};

#endif
