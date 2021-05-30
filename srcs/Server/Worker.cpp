#include "Worker.hpp"

namespace Webserv {

Worker::Worker(worker_config *w)
    : _id ("worker_" + w->id), _connections (ConnectionManagement(_id, w->conf))
{
    _w_conf = w;
    //_id = "worker_" + _w_conf->id;
    //_connections = ConnectionManagement(_id, _w_conf->conf);
    _connections._max_fd = 0;
    _connections._buffer = new char[8192];
    Log::out(_id, "created");
}

Worker::~Worker()
{
    pthread_mutex_unlock(&_w_conf->access);
    pthread_mutex_destroy(&_w_conf->access);
    delete[] _connections._buffer;
    FD_ZERO(&_connections._read_fds);
    FD_ZERO(&_connections._write_fds);
    Log::out(_id, "time in function: " + Time::get_total_time(start_time) + " - managed " + Utils::itoa(total_connections) + " connections.");
}

void    Worker::accept_new_connection()
{
    int new_client_socket = accept(_w_conf->tmp_connections, NULL, NULL);
    if (new_client_socket == -1) {
        Log::out(_id, std::string("new connection error: ") + std::strerror(errno));
    } else {
        FD_SET(new_client_socket, &_connections._write_fds);
        FD_SET(new_client_socket, &_connections._read_fds);
        _connections._max_fd = std::max(_connections._max_fd, new_client_socket);
        Log::out(_id, "new connection");
        ++_w_conf->nb_connections;
        ++total_connections;
    }
}

void    Worker::worker_routine()
{
    start_time = Time::get_now_time();
    total_connections = 0;
    FD_ZERO(&_connections._write_fds);
    FD_ZERO(&_connections._read_fds);
    _connections.lost_connections_count = 0;
    bool need_to_sleep;
    while (true)
    {
        pthread_mutex_lock(&_w_conf->access);
        _w_conf->nb_connections -= _connections.lost_connections_count;
        _connections.lost_connections_count = 0;
        if (_w_conf->stop)
            return ;
        if (_w_conf->new_connection) {
            accept_new_connection();
            _w_conf->new_connection = false;
        }
        need_to_sleep = _w_conf->nb_connections == 0;
        pthread_mutex_unlock(&_w_conf->access);
        if (_connections._max_fd)
            _connections.loop_worker();
        if (need_to_sleep)
            usleep(10000);
    }
}

void* launch_worker(void *w_config)
{
    Worker worker(reinterpret_cast<worker_config*>(w_config));
    worker.worker_routine();
    return (NULL);
}

};
