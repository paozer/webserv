#include "ServerManagement.hpp"

namespace Webserv {

static bool stop_server;

/*                 COMMUNICATION PART                 */

void                signal_handler(int sig)
{
    (void)sig;
    stop_server = true;
}

bool                wait_confirmation(int fd)
{
    char ret;
    read(fd, &ret, 1);
    return ret == READY;
}

void                send_confirmation(int fd, bool status)
{
    std::string res;
    res = status ? READY : ERROR;
    write(fd, res.c_str(), 1);
}

bool                send_msg(int pipe[2], std::string msg)
{
    write(pipe[OUT], msg.c_str(), msg.length());
    return (wait_confirmation(pipe[IN]));
}

std::string         receive_msg(int pipe[2])
{
    char            buffer[BUFFER_SIZE];
    std::string     res;

    bzero(buffer, BUFFER_SIZE);
    if (read(pipe[IN], &buffer, BUFFER_SIZE) < 0) {
        send_confirmation(pipe[OUT], false);
        return "error";
    } else {
        res = buffer;
        send_confirmation(pipe[OUT], true);
    }
    return res;
}

/*                      SERVER                       */

Server::Server(Configuration const &config)
     : _nb_worker(0), _configuration(config)
{
    ConnectionManagement connections("Server");
    _connections = connections;
    sockets_settings();
    signal(SIGINT, signal_handler);
    _workers = new pid_t[_configuration.max_workers];
    _pipes = new int* [_configuration.max_workers];
    for (int i = 0; i < _configuration.max_workers; ++i)
        _pipes[i] = new int[2];
    stop_server = false;
}

Server::~Server()
{
    for (int i = 0; i < _nb_worker; ++i)
    {
        send_msg(_pipes[i], EXIT);
        waitpid(_workers[i], NULL, 0);
        close(_pipes[i][IN]);
        close(_pipes[i][OUT]);
    }
    for (int i = 0; i < _configuration.max_workers; ++i)
        delete[] _pipes[i];
    delete[] _pipes;
    delete[] _workers;
    usleep(500);
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

int Server::new_worker(std::string const &socket_details)
{
    if (_nb_worker < _configuration.max_workers){
        int pipe_in[2];
        int pipe_out[2];
        pipe(pipe_in);
        pipe(pipe_out);
        _workers[_nb_worker] = fork();
        if (_workers[_nb_worker] == 0){
            Worker(_configuration, _nb_worker, pipe_in, pipe_out); // Child read in pipe in and receive in pipe out
            exit(EXIT_SUCCESS);
        }
        close(pipe_in[IN]);
        close(pipe_out[OUT]);
        _pipes[_nb_worker][IN] = pipe_out[IN];
        _pipes[_nb_worker][OUT] = pipe_in[OUT];
        send_msg(_pipes[_nb_worker], NEW_CONNECTION);
        send_msg(_pipes[_nb_worker], socket_details);
        if (receive_msg(_pipes[_nb_worker]) != CONNECTION_ACCEPTED)
            Log::out("server", "error socket");
        ++_nb_worker;
    } else {
        int     which = get_smaller_worker();
        send_msg(_pipes[which], NEW_CONNECTION);
        send_msg(_pipes[which], socket_details);
        if (receive_msg(_pipes[which]) != CONNECTION_ACCEPTED)
            Log::out("server", "error socket");
    }

    return (1);
}

int Server::get_smaller_worker()
{
    int     smaller_id;
    int     smaller_nb = INT_MAX;
    int     tmp = 0;

    for (int i = 0; i < _configuration.max_workers; ++i){
        send_msg(_pipes[i], GET_NB_CONNECTIONS);
        tmp = Utils::atoi(receive_msg(_pipes[i]).c_str());
        if (tmp < smaller_nb){
            smaller_nb = tmp;
            smaller_id = i;
        }
    }
    return (smaller_id);
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
            sleep(1);
        }
    }
}

};
