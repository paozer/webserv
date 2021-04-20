#ifndef CONNECTIONS_MANAGEMENT
# define CONNECTIONS_MANAGEMENT

#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>

#include <string>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <stdlib.h>
#include <signal.h>
#include <deque>

# include "Logger.hpp"
#include "Utils.hpp"
#include "ServerSocket.hpp"

namespace Webserv {

class ConnectionManagement
{
    public:
        ConnectionManagement();
        ConnectionManagement(std::string const &id);
        ConnectionManagement(ConnectionManagement const &other);
        ConnectionManagement& operator=(ConnectionManagement const &other);

        int     loop_worker();
        int     loop_server(std::vector<ServerSocket> const &serv_sock);
        bool    is_server_fd(const int current_fd, std::vector<ServerSocket> const &serv_sock);
        void    response_management(int current_fd);

        struct timeval  _tv;
        std::string     _id;
        std::string     _s_buffer;
        fd_set          _write_fds;
        fd_set          _read_fds;
        fd_set          _tmp_write_fds;
        fd_set          _tmp_read_fds;
        int             _max_fd;
        int             _nbytes;

        int             _nb_server;
};

};
#endif