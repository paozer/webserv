#ifndef CONNECTIONS_MANAGEMENT
# define CONNECTIONS_MANAGEMENT

#include <string>
#include <iostream>
#include <map>

#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <stdlib.h>
#include <signal.h>
#include <cstring>

#include "ServerSocket.hpp"
#include "Methods.hpp"
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/Utils.hpp"

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
        void    response_construction(int current_fd);
        void    response_sending(int fd);

        int             _max_fd;
        int             _nbytes;
        int             _nb_server;
        struct timeval  _tv;
        std::string     _id;
        std::string     _s_buffer;
        fd_set          _write_fds;
        fd_set          _read_fds;
        fd_set          _tmp_write_fds;
        fd_set          _tmp_read_fds;
        Configuration   _config;
        std::map<int, Http::Request>  _incomplete_request;
        std::map<int, std::list<std::string> > response_queue;
        std::map<int, std::pair<bool, bool> > response_condition;
};

}; // namespace Webserv
#endif
