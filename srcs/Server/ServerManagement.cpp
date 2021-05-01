#include "ServerManagement.hpp"

namespace Webserv {

static bool stop_server;

/*                 COMMUNICATION PART                 */

void                signal_handler(int sig)
{
    (void)sig;
    stop_server = true;
}

/*                      SERVER                       */

Server::Server(Configuration const &config)
     : _nb_worker(0), _configuration(config)
{
    ConnectionManagement connections("Server");
    _connections = connections;
    sockets_settings();
    signal(SIGINT, signal_handler);
    _workers = new worker_config[_configuration.max_workers];
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
    Log::out("Server", "stop");
    close(Log::fd);
}

void    Server::start()
{
    if (_configuration.max_workers == 0)
        main_loop_without_workers();
    else
        main_loop_with_workers();
}

void    Server::sockets_settings()
{
    _connections._max_fd = -1;
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

/*                 WORKER MANAGEMENT                  */

void Server::refused_connection(std::string const &socket_details)
{
    std::string msg;
    _cli_len = sizeof(_cli_addr);
    _cli_sock = accept(Utils::atoi(socket_details), reinterpret_cast<struct sockaddr*>(&_cli_addr), &_cli_len);
    msg = "connection refused, max is reached\n";
    send(_cli_sock, msg.c_str(), msg.length(), 0);
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
        pthread_create(&_workers[_nb_worker].th, NULL, launch_worker, &(_workers[_nb_worker]));
        // pthread_detach(_workers[_nb_worker].th);
        ++_nb_worker;
    } else {
        int which = get_smaller_worker();
        if (which != -1) {
            pthread_mutex_lock(&_workers[which].access);
            _workers[which].new_connection = true;
            _workers[which].tmp_connections = Utils::atoi(socket_details);
            pthread_mutex_unlock(&_workers[which].access);
            for (bool state = true; !state;) {
                pthread_mutex_lock(&_workers[which].access);
                state = _workers[which].new_connection;
                pthread_mutex_unlock(&_workers[which].access);
            }
        } else {
            refused_connection(socket_details);
        }
    }
}

int Server::get_smaller_worker()
{
    int     smaller_id;
    int     smaller_nb = INT_MAX;

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

/*                 SERVER PART                  */

void    Server::main_loop_without_workers()
{
    _cli_len = sizeof(_cli_addr);
    Log::out("Server", "start");
    _connections._config = _configuration;
    while (!stop_server)
    {
        _connections._tv.tv_sec = 0;
        _connections._tv.tv_usec = 500;
        _connections._tmp_read_fds = _connections._read_fds;
        _connections._tmp_write_fds = _connections._write_fds;
        select(_connections._max_fd + 1, &_connections._tmp_read_fds, &_connections._tmp_write_fds, NULL, &_connections._tv);
        for (size_t i = 0; i < _socket.size(); ++i)
        {
            if (FD_ISSET(_socket[i].get_fd(), &_connections._tmp_read_fds)) {
                _cli_sock = accept(_socket[i].get_fd(), reinterpret_cast<struct sockaddr*>(&_cli_addr), &_cli_len);
                if (_cli_sock == -1) {
                    Log::out("Server", "new connection error");
                } else {
                    FD_SET(_cli_sock, &_connections._read_fds);
                    FD_SET(_cli_sock, &_connections._write_fds);
                    _connections._max_fd = std::max(_connections._max_fd, _cli_sock);
                    Log::out("Server", "new connection");
                }
            }
        }
        _connections.loop_server(_socket);
    }
}

void    Server::main_loop_with_workers()
{
    fd_set      serv_socket;
    fd_set      tmp_socket;

    serv_socket = _connections._read_fds;
    FD_ZERO(&_connections._read_fds);
    Log::out("Server", "start");
    while (!stop_server)
    {
        tmp_socket = serv_socket;
        select(_connections._max_fd + 1, &tmp_socket, NULL, NULL, NULL);
        for (size_t i = 0; i < _socket.size(); ++i)
        {
            if (FD_ISSET(_socket[i].get_fd(), &tmp_socket)){
                new_worker(Utils::itoa(_socket[i].get_fd()));
                FD_CLR(_socket[i].get_fd(), &tmp_socket);
            }
        }
            // sleep(1);
    }
}

};
