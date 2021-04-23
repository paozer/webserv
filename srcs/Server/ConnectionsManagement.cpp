#include "ConnectionsManagement.hpp"

#include <cerrno>
#include <iostream>
#include <cmath>
#include <cstring>
#include <clocale>

namespace Webserv {

ConnectionManagement::ConnectionManagement() {}

ConnectionManagement::ConnectionManagement(std::string const &id) : _id(id) {}

ConnectionManagement::ConnectionManagement(ConnectionManagement const &other) { *this = other;}

ConnectionManagement& ConnectionManagement::operator=(ConnectionManagement const &other)
{
    if (this != &other) {
        _id = other._id;
        _write_fds = other._write_fds;
        _max_fd = other._max_fd;
    }
    return *this;
}

int     ConnectionManagement::loop_server(std::vector<ServerSocket> const &serv_sock)
{
    char        buffer[BUFFER_SIZE + 1];
    int         connections_loose = 0;

    for (int i = 0; i <= _max_fd; ++i) {
        if (!is_server_fd(i, serv_sock) && FD_ISSET(i, &_tmp_read_fds)) {
            bzero(buffer, BUFFER_SIZE + 1);
            _nbytes = recv(i, buffer, BUFFER_SIZE, 0);
             if (_nbytes <= 0) {
                _nbytes == 0 ? Log::out(_id, "connection dropped") : Log::out(_id, "recv error");
                close(i);
                FD_CLR(i, &_write_fds);
                FD_CLR(i, &_read_fds);
                ++connections_loose;
            } else {
                _s_buffer = std::string(buffer, _nbytes);
                Log::out(_id, "reception: " +_s_buffer);
                response_management(i);
            }
        }
    }
    return connections_loose;
}

int    ConnectionManagement::loop_worker()
{
    char        buffer[BUFFER_SIZE + 1];
    int         connections_loose = 0;

    _tv.tv_sec = 0;
    _tv.tv_usec = 500;
    _tmp_read_fds = _read_fds;
    _tmp_write_fds = _write_fds;
    select(_max_fd + 1, &_tmp_read_fds, &_tmp_write_fds, NULL, &_tv);
    for (int i = 0; i <= _max_fd; ++i) {
        if (FD_ISSET(i, &_tmp_read_fds)){
            bzero(buffer, BUFFER_SIZE + 1);
            _nbytes = recv(i, buffer, BUFFER_SIZE, 0);
             if (_nbytes <= 0) {
                _nbytes == 0 ? Log::out(_id, "connection dropped") : Log::out(_id, "recv error");
                close(i);
                FD_CLR(i, &_write_fds);
                FD_CLR(i, &_read_fds);
                ++connections_loose;
            } else {
                _s_buffer = std::string(buffer, _nbytes);
                Log::out(_id, "reception: " +_s_buffer);
                response_management(i);
            }
        }
    }
    return connections_loose;
}

void    ConnectionManagement::response_management(int current_fd)
{
    // _incomplete_request[current_fd].append(_s_buffer);
    // Http::Request::State state = _incomplete_request[current_fd].get_state();
    // if (state == Http::Request::Complete || state == Http::Request::Error) {
    //     Http::Response response;
    //     _incomplete_request[current_fd].parse_raw_packet(_s_buffer);
    //     response = Methods::method_handler(_incomplete_request[current_fd], _config);
    //     response.build_raw_packet();
    //     _s_buffer = response.get_raw_packet();
    _s_buffer = "good questions.\n";
        if (FD_ISSET(current_fd, &_tmp_write_fds))
            send(current_fd, _s_buffer.c_str(), _s_buffer.size(), 0);
        _incomplete_request.erase(current_fd);
    // }
}

bool    ConnectionManagement::is_server_fd(const int current_fd, std::vector<ServerSocket> const &serv_sock)
{
    for (size_t i = 0; i < serv_sock.size(); ++i)
        if (serv_sock[i].get_fd() == current_fd)
            return true;
    return false;
}

};
