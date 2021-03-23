#include "ServerSocket.hpp"

namespace Webserv {

ServerSocket::ServerSocket(const std::string& ip_address, int port)
{
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd == -1) {
        perror("socket");
        exit(-1);
    }

    if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        close(_socket_fd);
        exit(-1);
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    sock_addr.sin_port = htons(port);
    bzero(&sock_addr.sin_zero, 8);

    if (sock_addr.sin_addr.s_addr == -1) {
        perror("inet_addr");
        close(_socket_fd);
        exit(-1);
    }

    if (bind(_socket_fd, reinterpret_cast<struct sockaddr*>(&sock_addr), sizeof(sock_addr)) == -1) {
        perror("bind");
        close(_socket_fd);
        exit(-1);
    }

    if (listen(_socket_fd, 20) == -1) {
        perror("listen");
        close(_socket_fd);
        exit(-1);
    }
}

int ServerSocket::get_socket(void)
{
    return _socket_fd;
}

void ServerSocket::set_socket(int socket_fd)
{
    _socket_fd = socket_fd;
}

}; // namespace Webserv
