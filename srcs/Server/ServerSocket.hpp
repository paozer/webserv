#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

#include <string>

#include <sys/socket.h> // socket, accept, bind
#include <arpa/inet.h> // inet_addr
#include <fcntl.h> // fcntl
#include <unistd.h> // close
#include <errno.h>
#include <string.h>

namespace Webserv {

class ServerSocket {
    public:
        int         set_socket_fd (const std::string& ip_address, int port);
        inline int  get_fd(void) const { return _socket_fd; }

    private:
        int _socket_fd;

}; // class ServerSocket

}; // namespace Webserv

# endif
