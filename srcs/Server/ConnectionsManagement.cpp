#include "ConnectionsManagement.hpp"

namespace Webserv {

ConnectionManagement::ConnectionManagement() {}

ConnectionManagement::ConnectionManagement(std::string const &id, const Configuration &config)
    : _max_fd(0), lost_connections_count(0), _id(id), _config(config)
{
    _buffer = new char[8192];
}

ConnectionManagement::ConnectionManagement(ConnectionManagement const &other) { *this = other;}

ConnectionManagement& ConnectionManagement::operator=(ConnectionManagement const &other)
{
    if (this != &other) {
        _id = other._id;
        _write_fds = other._write_fds;
        _read_fds = other._read_fds;
        _max_fd = other._max_fd;
        _config = other._config;
    }
    return *this;
}

int     ConnectionManagement::loop_server(std::vector<ServerSocket> const &serv_sock)
{
    lost_connections_count = 0;
    for (int i = 0; i <= _max_fd; ++i) {
        if (!is_server_fd(i, serv_sock)) {
            if (FD_ISSET(i, &_tmp_read_fds))
                handle_incoming(i);
            if (next_step && FD_ISSET(i, &_tmp_write_fds))
                send_response(i);
        }
    }
    return lost_connections_count;
}

int    ConnectionManagement::loop_worker()
{
    _timeout.tv_sec = 0;
    _timeout.tv_usec = 200;
    _tmp_read_fds = _read_fds;
    _tmp_write_fds = _write_fds;
    lost_connections_count = 0;
    select(_max_fd + 1, &_tmp_read_fds, &_tmp_write_fds, NULL, &_timeout);
    for (int i = 0; i <= _max_fd; ++i) {
        if (FD_ISSET(i, &_tmp_read_fds))
            handle_incoming(i);
        if (next_step && FD_ISSET(i, &_tmp_write_fds))
            send_response(i);
    }
    return lost_connections_count;
}

void ConnectionManagement::handle_incoming (int fd)
{
    int ret = recv(fd, _buffer, 8192, 0);
    next_step = true;
    if (ret > 0) {
        _s_buffer = std::string(_buffer, ret);
        construct_response(fd);
        return ;
    }
    if (ret == 0)
        Log::out(_id, "connection dropped");
    else
        Log::out(_id, std::string("recv error: ") + strerror(errno));
    _incomplete_request.erase(fd);
    close_connection(fd);
    next_step = false;
}

void    ConnectionManagement::send_response (int fd)
{
    struct settings *tmp = &ready_responses[fd];
    if (tmp->response_queue.empty())
        return ;
    size_t length = std::min(tmp->response_queue.length() - tmp->offset, static_cast<size_t>(8192));
    int ret = write(fd, tmp->response_queue.c_str() + tmp->offset, length);
    if (ret >= 0) {
        tmp->offset += ret;
        if (tmp->offset == static_cast<long>(tmp->response_queue.length())) {
            if (tmp->should_close)
                close_connection(fd);
            else
                ready_responses.erase(fd);
        }
    } else {
        close_connection(fd);
    }
}

void    ConnectionManagement::construct_response(int fd)
{
    Http::Request& request = _incomplete_request[fd];
    request.append(_s_buffer);
    Http::State state = request.get_state();
    if (state == Http::Complete || state == Http::Error) {
        Http::Response response = Methods::method_handler(request, _config, fd);
        response.build_raw_packet();
        ready_responses[fd] = settings(response.should_close(), 0, response.get_raw_packet());
        _incomplete_request.erase(fd);
    }
}

void ConnectionManagement::close_connection (int fd)
{
    ++lost_connections_count;
    close(fd);
    FD_CLR(fd, &_write_fds);
    FD_CLR(fd, &_read_fds);
    ready_responses.erase(fd);
    if (fd == _max_fd) {
        while (_max_fd > 0 && FD_ISSET(_max_fd, &_read_fds) == false)
            --_max_fd;
    }
}

bool    ConnectionManagement::is_server_fd(int fd, std::vector<ServerSocket> const &serv_sock)
{
    for (size_t i = 0; i < serv_sock.size(); ++i)
        if (serv_sock[i].get_fd() == fd)
            return true;
    return false;
}

};
