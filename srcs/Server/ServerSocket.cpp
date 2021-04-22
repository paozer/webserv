#include "ServerSocket.hpp"

namespace Webserv {

int ServerSocket::set_socket_fd(const std::string& ip_address, int port)
{
    if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        strerror(errno);
        return -1;
    }

    if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) == -1) {
        strerror(errno);
        close(_socket_fd);
        return -1;
    }

	int opt = 1;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		strerror(errno);
		close(_socket_fd);
        return -1;
	}

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    sock_addr.sin_port = htons(port);
    bzero(&sock_addr.sin_zero, 8);
    if (sock_addr.sin_addr.s_addr == INADDR_NONE) {
        strerror(errno);
        close(_socket_fd);
        return -1;
    }

    if (bind(_socket_fd, reinterpret_cast<struct sockaddr*>(&sock_addr), sizeof(sock_addr)) == -1) {
        strerror(errno);
        close(_socket_fd);
        return -1;
    }

    if (listen(_socket_fd, 20) == -1) {
        strerror(errno);
        close(_socket_fd);
        return -1;
    }

    return 0;
}

}; // namespace Webserv
