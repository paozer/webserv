#ifndef SERVER_MANAGEMENT
# define SERVER_MANAGEMENT

#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>

#include <string>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#if defined(__linux__)
	#include <bits/stdc++.h>
#endif

#if defined(__linux__)
	#include <bits/stdc++.h>
#endif

#include "ConnectionsManagement.hpp"
#include "ServerSocket.hpp"
#include "../Configuration/Parsing.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Utils.hpp"
#include "../Utils/Logger.hpp"

# define IN 0
# define OUT 1
# define READY 'r'
# define ERROR 'e'
# define NEW_CONNECTION "1"
# define GET_NB_CONNECTIONS "2"
# define CONNECTION_ACCEPTED "3"
# define EXIT "-1"

namespace Webserv {

void            signal_handler(int sig);
void            *launch_worker(void *w);

struct          worker_config;

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

        std::vector<ServerSocket>  _socket;

        struct sockaddr_in      _cli_addr;
        unsigned int            _cli_len;
        int                     _cli_sock;

        worker_config           *_workers;

        void                    main_loop_with_workers();
        void                    main_loop_without_workers();
        void                    sockets_settings();
        int                     get_smaller_worker();
        void                    new_worker(std::string const &socket_details);
        void                    refused_connection(std::string const &socket_details);
};

struct  worker_config
{
    worker_config() : nb_connections(0), tmp_connections(0), new_connection(false), stop(false) {}
    int             nb_connections;
    int             tmp_connections;
    bool            new_connection;
    bool            stop;
    Configuration   conf;
    pthread_t       th;
    pthread_mutex_t access;
    std::string     id;
};

class Worker
{

    public:
        Worker(worker_config *w);
        ~Worker();
        Worker(const Worker &other);
        Worker& operator=(const Worker &other);
        void                    worker_routine();

    private:
        std::string             _id;
        std::string             _s_buffer;

        struct sockaddr_in      _cli_addr;
        unsigned int            _cli_len;

        Configuration           _configuration;
        ConnectionManagement    _connections;
        worker_config           *_w_conf;

        void                    accept_new_connection();
        void                    get_new_connection_fd();
        void                    send_num_connections();
        void                    stop_worker();
        static void             *worker_thread(void *worker);
};

};

#endif
