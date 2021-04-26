#include "ServerManagement.hpp"

namespace Webserv {

/*                 START / STOP WORKER                  */

void* launch_worker(void *w)
{
    Worker worker(reinterpret_cast<worker_config*>(w));
    worker.worker_routine();
    return (NULL);
}

Worker::Worker(worker_config *w)
{
    _w_conf = w;
    _id = "worker_" + _w_conf->id;
    _connections = ConnectionManagement(_id);
    _connections._config = _w_conf->conf;
    _connections._max_fd = 0;
    _cli_len = sizeof(_cli_addr);
    Log::out(_id, "created");
}

Worker::~Worker()
{
    pthread_mutex_unlock(&_w_conf->access);
    pthread_mutex_destroy(&_w_conf->access);
    FD_ZERO(&_connections._read_fds);
    FD_ZERO(&_connections._write_fds);
    Log::out(_id, "ciao!");
}

/*                 MAIN PART                  */
/*          (listen port and response)        */

void    Worker::accept_new_connection()
{
    int         new_client_socket;

    new_client_socket = accept(_w_conf->tmp_connections, reinterpret_cast<struct sockaddr*>(&_cli_addr), &_cli_len);
    if (new_client_socket == -1) {
        Log::out(_id, "new connection error");
    } else {
        FD_SET(new_client_socket, &_connections._write_fds);
        FD_SET(new_client_socket, &_connections._read_fds);
        _connections._max_fd = std::max(_connections._max_fd, new_client_socket);
        Log::out(_id, "new connection");
        ++_w_conf->nb_connections;
    }
}

void    Worker::worker_routine()
{
    int connections_loose = 0;
    FD_ZERO(&_connections._write_fds);
    FD_ZERO(&_connections._read_fds);
    while(true)
    {
        // std::cout << "ok" << std::endl;
        pthread_mutex_lock(&_w_conf->access);
        if (_w_conf->stop)
            return ;
        if (_w_conf->new_connection){
            accept_new_connection();
            _w_conf->new_connection = false;
        }
        _w_conf->nb_connections -= connections_loose;
        pthread_mutex_unlock(&_w_conf->access);
        connections_loose = 0;
        if (_w_conf->nb_connections) {
            connections_loose = _connections.loop_worker();
        }
    }
}

};
