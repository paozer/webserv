#include "ServerSocket.hpp"

namespace Webserv {

int ServerSocket::set_socket_fd(const std::string& ip_address, int port)
{
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        strerror(errno);
        return -1;
    }

    int opt = 1;
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1 ||
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        strerror(errno);
        close(fd);
        return -1;
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    sock_addr.sin_port = htons(port);
    bzero(&sock_addr.sin_zero, 8);
    if (sock_addr.sin_addr.s_addr == INADDR_NONE) {
        strerror(errno);
        close(fd);
        return -1;
    }

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&sock_addr), sizeof(sock_addr)) == -1 ||
            listen(fd, 1000) == -1) {
        strerror(errno);
        close(fd);
        return -1;
    }
    return 0;
}

}; // namespace Webserv
