#include "ConnectionsManagement.hpp"

namespace Webserv {
int q;
int w;

ConnectionManagement::ConnectionManagement(std::string const &id, const Configuration &config)
    : _max_fd(0), lost_connections_count(0), _id(id), _config(config)
{q = 0;}

int     ConnectionManagement::loop_server(std::vector<ServerSocket> const &serv_sock)
{
    lost_connections_count = 0;
    for (int i = 0; i <= _max_fd; ++i) {
        if (!is_server_fd(i, serv_sock)) {
            if (FD_ISSET(i, &_tmp_read_fds))
                handle_incoming(i);
            if (FD_ISSET(i, &_tmp_write_fds))
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
        if (FD_ISSET(i, &_tmp_write_fds))
            send_response(i);
    }
    return lost_connections_count;
}

void ConnectionManagement::handle_incoming (int fd)
{
    int ret = recv(fd, _buffer, 8192, 0);
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
}

void    ConnectionManagement::send_response (int fd)
{
    struct settings *tmp = &ready_responses[fd];
    if (!(tmp->response_queue.empty())) {
        size_t length = std::min(tmp->response_queue.length() - tmp->offset, static_cast<size_t>(8192));
        int ret = write(fd, tmp->response_queue.c_str() + tmp->offset, length);
        if (ret >= 0) {
            tmp->offset += ret;
            if (tmp->offset == static_cast<long>(tmp->response_queue.length())) {
                if (tmp->tmpFileFd == -1) {
                    if (tmp->should_close)
                        close_connection(fd);
                    else
                        ready_responses.erase(fd);
                } else {
                    tmp->readfile = true;
                    tmp->response_queue.clear();
                }
            }
        } else {
            close_connection(fd);
        }
    } else if (tmp->readfile) {
        char buffer[8192];
        int ret_read = read(tmp->tmpFileFd, buffer, 8192);
        q += ret_read;
        std::cerr << "retour read : " << ret_read << " tot = " << q << std::endl;
        if (ret_read < 0) { // Erreur read
            sleep(2);
            Log::out(_id + "(read)", std::strerror(errno));
        } else {
            int ret_write = write(fd, buffer, ret_read);
            w += ret_write;
            std::cerr << "retour write : " << ret_write << " tot = " << w << std::endl;
            if (ret_read < 8192 && ret_read == ret_write) { // Tout s'est bien passé et c'est fini
                close(tmp->tmpFileFd);
                if (tmp->should_close)
                    close_connection(fd);
                ready_responses.erase(fd);
                tmp->readfile = false;
                std::cerr << "finish" << std::endl;
                sleep(2);
            } else if (ret_write >= 0 && ret_write < ret_read) { // bien envoyé mais différence entre lu et envoyé
                lseek(tmp->tmpFileFd, ret_write - ret_read, SEEK_CUR);
            } else if (ret_write < 0) { // Erreur write
                close(tmp->tmpFileFd);
                close_connection(fd);
            } // Tout s'est bien passé et reste du contenu, on fait rien de spécial
        }
        // sleep(1);
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
        ready_responses[fd] = settings(response.should_close(), 0, response.get_raw_packet(), response.file_fd);
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

void    ConnectionManagement::close_tmp_file(void)
{
    std::map<int, struct settings>::iterator it = ready_responses.begin();
    for (; it != ready_responses.end(); ++it)
        if ((*it).second.tmpFileFd != -1)
            close((*it).second.tmpFileFd);
}

bool    ConnectionManagement::is_server_fd(int fd, std::vector<ServerSocket> const &serv_sock)
{
    for (size_t i = 0; i < serv_sock.size(); ++i)
        if (serv_sock[i].get_fd() == fd)
            return true;
    return false;
}

};
