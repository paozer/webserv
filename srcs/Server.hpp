#pragma once

#include "ServerSocket.hpp"

#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <sys/types.h>

#include <iostream>

namespace Webserv {

class Server
{
    public:
        Server();
        void start (void);
        void stop (void);

    private:
        ServerSocket _socket;
        int main_cycle(void);

}; // class Server

}; // namespace Webserv
