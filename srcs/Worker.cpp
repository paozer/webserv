#include "Server_management.hpp"

namespace Webserv {

/*                 START / STOP WORKER                  */

Worker::Worker(Configuration const &config, int id, int pipe_in[2], int pipe_out[2])
    : _configuration(config)
{

    _w_conf = new worker_config(0, 0, false, "worker_" + Utils::itoa(id));
    _w_conf->stop = false;
    _cli_len = sizeof(_cli_addr);
    close(pipe_in[OUT]);
    close(pipe_out[IN]);
    _pipe[IN] = pipe_in[IN];
    _pipe[OUT] = pipe_out[OUT];
    Log::out(_w_conf->id, "created");
    pthread_mutex_init(&_m_new, NULL);
    pthread_create(&_th, NULL, worker_thread, reinterpret_cast<void*>(this));
    worker_routine();
}


Worker::Worker(const Worker &other) { *this =  other; }

Worker& Worker::operator=(const Worker &other)
{
    if (this != &other) {
        _pipe[0] = other._pipe[0];
        _pipe[1] = other._pipe[1];
        _w_conf = other._w_conf;
    }
    return *this;
}

Worker::~Worker()
{
    pthread_mutex_destroy(&_m_new);
    // close(_pipe[IN]);
    // close(_pipe[OUT]);
}
/*                 THREAD PART                  */
/*          (communication with server)         */

void    Worker::get_new_connection_fd()
{
    _s_buffer = receive_msg(_pipe);
    pthread_mutex_lock(&_m_new);
    _w_conf->tmp_connections = Utils::atoi(_s_buffer.c_str());
    _w_conf->new_connection = true;
    pthread_mutex_unlock(&_m_new);
    while (_w_conf->new_connection);
    send_msg(_pipe, CONNECTION_ACCEPTED);
}

void    Worker::send_num_connections()
{
    pthread_mutex_lock(&_m_new);
    _s_buffer = Utils::itoa(_w_conf->nb_connections);
    pthread_mutex_unlock(&_m_new);
    send_msg(_pipe, _s_buffer);
}

void    Worker::stop_worker()
{
    _w_conf->stop = true;
    Log::out(_id, "Exit thread's routine");
    close(_pipe[IN]);
    close(_pipe[OUT]);
    exit(EXIT_SUCCESS);
}

void    *Worker::worker_thread(void *worker)
{
    Worker  w = *(reinterpret_cast<Worker*>(worker));
    w._id = w._w_conf->id;
    while (true)
    {
        w._s_buffer = receive_msg(w._pipe);
        if (w._s_buffer == NEW_CONNECTION){
            w.get_new_connection_fd();
        } else if (w._s_buffer == GET_NB_CONNECTIONS) {
            w.send_num_connections();
        } else if (w._s_buffer == EXIT) {
            w.stop_worker();
            break ;
        }
    }
    return (EXIT_SUCCESS);
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
    }
    ++_w_conf->nb_connections;
}

void    Worker::worker_routine()
{
    int connections_loose = 0;
    _id = _w_conf->id;
    _connections = ConnectionManagement(_id);
    _connections._config = _configuration;
    _connections._max_fd = 0;
    FD_ZERO(&_connections._write_fds);
    FD_ZERO(&_connections._read_fds);
    while(true)
    {
        if (_w_conf->stop){
            delete _w_conf;
            Log::out(_id, "Exit Worker's routine");
            exit(EXIT_SUCCESS);
        }
        pthread_mutex_lock(&_m_new);
        if (_w_conf->new_connection){
            accept_new_connection();
            _w_conf->new_connection = false;
        }
        _w_conf->nb_connections -= connections_loose;
        pthread_mutex_unlock(&_m_new);
        if (_w_conf->nb_connections) {
            connections_loose = _connections.loop_worker();
        }
    }
}

};
