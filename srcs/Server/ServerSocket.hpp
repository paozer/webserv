#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include "../Http/Http.hpp"

#include <string>

#include <sys/socket.h> // socket, accept, bind
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <fcntl.h> // fcntl
#include <errno.h> // errno
#include <unistd.h> // close

#include <string.h>

namespace Webserv {

class ServerSocket {
    public:
        int set_socket_fd (const std::string& ip_address, int port);
        inline int get_fd(void) const { return fd; }

    private:
        int fd;

}; // class ServerSocket

}; // namespace Webserv

# endif
