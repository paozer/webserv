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
#include <deque>

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

std::string     receive_msg(int pipe[2]);
bool            send_msg(int pipe[2], std::string msg);
bool            wait_confirmation(int fd);
void            send_confirmation(int fd, bool status);
void            signal_handler(int sig);

class Server
{
    public:
        Server(Configuration const &config);
        ~Server();
        void start();

    private:
        int                     _nb_worker;
        int                     **_pipes;
        std::string             _s_buffer;

        pid_t                   *_workers;
        Configuration           _configuration;
        ConnectionManagement    _connections;

        std::vector<ServerSocket>  _socket;

        struct sockaddr_in      _cli_addr;
        unsigned int            _cli_len;
        int                     _cli_sock;

        void                    main_loop_with_workers();
        void                    main_loop_without_workers();
        void                    sockets_settings();
        int                     get_smaller_worker();
        int                     new_worker(std::string const &socket_details);
        void                    refused_connection(std::string const &socket_details);
};

struct  worker_config
{
    worker_config(int nb, int tmp, bool new_co, std::string w_id)
        : nb_connections(nb), tmp_connections(tmp), new_connection(new_co), id(w_id) {}
    int             nb_connections;
    int             tmp_connections;
    bool            new_connection;
    bool            stop;
    std::string     id;
};

class Worker
{
    typedef void * (*THREADFUNCPTR)(void *);

    public:
        Worker(Configuration const &config, int id, int pipe_in[2], int pipe_out[2]);
        ~Worker();
        Worker(const Worker &other);
        Worker& operator=(const Worker &other);

    private:
        int                     _pipe[2];
        std::string             _id;
        std::string             _s_buffer;

        struct sockaddr_in      _cli_addr;
        unsigned int            _cli_len;

        pthread_t               _th;
        pthread_mutex_t         _m_new;

        Configuration           _configuration;
        ConnectionManagement    _connections;
        worker_config           *_w_conf;

        void                    worker_routine();
        void                    accept_new_connection();
        void                    get_new_connection_fd();
        void                    send_num_connections();
        void                    stop_worker();
        static void             *worker_thread(void *worker);
};

};

#endif