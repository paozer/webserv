#ifndef WORKER_HPP
# define WORKER_HPP

#include <string>
#include <vector>
#include <unistd.h> // close, unlink

#include "ConnectionsManagement.hpp"
#include "ServerSocket.hpp"
#include "../Configuration/Parsing.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Utils.hpp"
#include "../Utils/Logger.hpp"

#include <cstring>

namespace Webserv {

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
        void worker_routine();

    private:
        std::string             _id;
        std::string             _s_buffer;

        Configuration           _configuration;
        ConnectionManagement    _connections;
        worker_config           *_w_conf;

        int                     total_connections;
        time_t                  start_time;

        void                    accept_new_connection();
        void                    stop_worker();
        static void             *worker_thread(void *worker);
};

void *launch_worker(void *w);

};

#endif
