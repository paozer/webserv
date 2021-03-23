#pragma once

#include <sys/socket.h> // socket, accept, bind
#include <arpa/inet.h> // inet_addr
#include <fcntl.h> // fcntl
#include <unistd.h> // close
#include <strings.h> // bzero

namespace Webserv {

class ServerSocket {
    public:
        ServerSocket(const std::string& ip_address, int port);
        int get_socket(void);
        void set_socket(int socket_fd)

    private:
        int _socket_fd;

}; // class ServerSocket

}; // namespace Webserv
