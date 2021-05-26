#include "ServerManagement.hpp"

namespace Webserv {

Server::Server(Configuration const &config)
     : _nb_worker(0), _configuration(config), _connections (ConnectionManagement("Server", config))
{
    sockets_settings();
    signal(SIGINT, signal_handler);
    _workers = new worker_config[_configuration.max_workers];
    total_connections = 0;
    start_time = Time::get_now_time();
}

Server::~Server()
{
    for (int i = 0; i < _nb_worker; ++i) {
        pthread_mutex_lock(&_workers[i].access);
        _workers[i].stop = true;
        pthread_mutex_unlock(&_workers[i].access);
    }
    for (int i = 0; i < _nb_worker; ++i)
        pthread_join(_workers[i].th, NULL);
    delete[] _workers;
    if (!_configuration.max_workers)
        delete[] _connections._buffer;
    Files::remove_dir("tmpFile");
    Log::out("Server", "stop");
    Log::out("", " total connections: " + Utils::itoa(total_connections));
    Log::out("", " server uptime: " + Time::get_total_time(start_time));
    close(Log::fd);
}

void    Server::start()
{
    if (_configuration.max_workers == 0) {
        _connections._buffer = new char[8192];
        main_loop_without_workers();
    } else {
        main_loop_with_workers();
    }
}

void    Server::sockets_settings()
{
    FD_ZERO(&_connections._write_fds);
    FD_ZERO(&_connections._read_fds);
    for (size_t i = 0; i < _configuration.get_nb_server(); ++i)
    {
        ServerSocket sock;
        sock.set_socket_fd(_configuration.get_server(i)->_listen.second, _configuration.get_server(i)->_listen.first);
        _socket.push_back(sock);
        FD_SET(_socket.back().get_fd(), &_connections._read_fds);
        if (_socket.back().get_fd() > _connections._max_fd)
            _connections._max_fd = _socket.back().get_fd();
    }
}

/*                 SERVER PART                  */

void    Server::main_loop_without_workers()
{
    Log::out("Server", "start");
    while (!stop_server)
    {
        _connections._timeout.tv_sec = 0;
        _connections._timeout.tv_usec = 500;
        _connections._tmp_read_fds = _connections._read_fds;
        _connections._tmp_write_fds = _connections._write_fds;
        select(_connections._max_fd + 1, &_connections._tmp_read_fds, &_connections._tmp_write_fds, NULL, &_connections._timeout);
        for (size_t i = 0; i < _socket.size(); ++i)
        {
            if (FD_ISSET(_socket[i].get_fd(), &_connections._tmp_read_fds)) {
                _cli_sock = accept(_socket[i].get_fd(), NULL, NULL);
                if (_cli_sock == -1) {
                    Log::out("Server", "new connection error");
                } else {
                    FD_SET(_cli_sock, &_connections._read_fds);
                    FD_SET(_cli_sock, &_connections._write_fds);
                    _connections._max_fd = std::max(_connections._max_fd, _cli_sock);
                    ++total_connections;
                    Log::out("Server", "new connection");
                }
            }
        }
        _connections.loop_server(_socket);
        usleep(5000);
    }
}

void    Server::main_loop_with_workers()
{
    fd_set tmp_socket;
    Log::out("Server", "start");
    while (!stop_server)
    {
        tmp_socket = _connections._read_fds;
        select(_connections._max_fd + 1, &tmp_socket, NULL, NULL, NULL);
        for (size_t i = 0; i < _socket.size(); ++i)
            if (FD_ISSET(_socket[i].get_fd(), &tmp_socket)) {
                new_worker(Utils::itoa(_socket[i].get_fd()));
                FD_CLR(_socket[i].get_fd(), &tmp_socket);
            }
    }
}

/*                 WORKER MANAGEMENT                  */

void Server::refused_connection(std::string const &socket_details)
{
    _cli_sock = accept(Utils::atoi(socket_details), NULL, NULL);
    send(_cli_sock, "connection refused, max is reached\n", 35, 0);
    close(_cli_sock);
    Log::out("server", "new connection refused, the maximum number of connections is reached");
}

void Server::new_worker(std::string const &socket_details)
{
    if (_nb_worker < _configuration.max_workers){
        pthread_mutex_init(&_workers[_nb_worker].access, NULL);
        _workers[_nb_worker].id = Utils::itoa(_nb_worker);
        _workers[_nb_worker].new_connection = true;
        _workers[_nb_worker].tmp_connections = Utils::atoi(socket_details);
        _workers[_nb_worker].conf = _configuration;
        _workers[_nb_worker].nb_connections = 0;
        pthread_create(&_workers[_nb_worker].th, NULL, launch_worker, &(_workers[_nb_worker]));
        for (bool state = true; state; )
        {
            pthread_mutex_lock(&_workers[_nb_worker].access);
            state = _workers[_nb_worker].new_connection;
            pthread_mutex_unlock(&_workers[_nb_worker].access);
        }
        ++_nb_worker;
        ++total_connections;
    } else {
        int i = get_available_worker();
        if (i != -1) {
            pthread_mutex_lock(&_workers[i].access);
            _workers[i].new_connection = true;
            _workers[i].tmp_connections = Utils::atoi(socket_details);
            pthread_mutex_unlock(&_workers[i].access);
            for (bool state = true; state; )
            {
                pthread_mutex_lock(&_workers[i].access);
                state = _workers[i].new_connection;
                pthread_mutex_unlock(&_workers[i].access);
            }
            ++total_connections;
        } else {
            refused_connection(socket_details);
        }
    }
}

int Server::get_available_worker()
{
    int smaller_id;
    int smaller_nb = INT_MAX;

    for (int i = 0; i < _configuration.max_workers; ++i){
        pthread_mutex_lock(&_workers[i].access);
        if (_workers[i].nb_connections < smaller_nb) {
            smaller_nb = _workers[i].nb_connections;
            smaller_id = i;
        }
        pthread_mutex_unlock(&_workers[i].access);
    }
    return smaller_nb < _configuration.max_connections_workers ? smaller_id : -1;
}

void signal_handler(int sig)
{
    (void)sig;
    stop_server = true;
}

};
