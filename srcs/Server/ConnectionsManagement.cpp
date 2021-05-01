#include "ConnectionsManagement.hpp"

#include <iostream>

#include <cerrno>
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
                // Log::out(_id, _s_buffer);
                response_construction(i);
            }
        }
        response_sending(i);
    }
    return connections_loose;
}

int    ConnectionManagement::loop_worker()
{
    char        buffer[BUFFER_SIZE + 1];
    int         connections_loose = 0;

    _tv.tv_sec = 0;
    _tv.tv_usec = 200;
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
                response_construction(i);
            }
        }
        response_sending(i);
    }
    return connections_loose;
}

void    ConnectionManagement::response_sending(int fd)
{
    if (FD_ISSET(fd, &_tmp_write_fds) && !response_queue[fd].empty()){
        send(fd, response_queue[fd].front().c_str(), response_queue[fd].front().size(), 0);
        if (response_condition[fd].first == true) {
            usleep(200);
            std::string msg;
            if (response_condition[fd].second == true) {
                msg = "Start sending " + Utils::itoa(response_queue[fd].size()) + " chunks to " + Utils::itoa(fd) + ".";
                Log::out(_id, msg);
                // Log::out(_id, response_queue[fd].front().substr(0, 50).c_str());
                response_condition[fd].second = false;
            } else if (response_queue[fd].front().size() != 60000) {
                msg = "End of sending to " + Utils::itoa(fd) + ".";
                Log::out(_id, msg);
            }
        } else {
            Log::out(_id, "sending response to " + Utils::itoa(fd));
        }
        response_queue[fd].pop_front();
    }
}

void    ConnectionManagement::response_construction(int fd)
{
    Http::Request& request = _incomplete_request[fd];
    request.append(_s_buffer, 300000000); // TODO max_client_body_size
    Http::State state = request.get_state();
    if (state == Http::Complete || state == Http::Error) {
        Http::Response response;
        response = Methods::method_handler(request, _config, fd);
        if (request.get_state() != Http::Error && request.get_method() != "HEAD")
            response.set_content_length();
        response.build_raw_packet();
        if (response.get_raw_packet().length() > 60000) {
            long length = response.get_raw_packet().length();
            for (long idx = 0; idx < length; idx += 60000)
                response_queue[fd].push_back(response.get_raw_packet().substr(idx, 60000));
            response_condition[fd] = std::make_pair(true, true);
        } else {
            response_queue[fd].push_back(response.get_raw_packet());
            response_condition[fd] = std::make_pair(false, false);
        }
        _incomplete_request.erase(fd);
    }
}

bool    ConnectionManagement::is_server_fd(const int current_fd, std::vector<ServerSocket> const &serv_sock)
{
    for (size_t i = 0; i < serv_sock.size(); ++i)
        if (serv_sock[i].get_fd() == current_fd)
            return true;
    return false;
}

};
