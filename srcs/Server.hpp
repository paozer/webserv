#pragma once

#include "ServerSocket.hpp"
#include "Parsing.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>

namespace Webserv {

class Server
{
    public:
        Server(Configuration::server const &config);
        ~Server();
        void start (void);
        void stop (void);

    private:
        Server();
        int main_cycle(void);

        ServerSocket            _socket;
        Configuration::server   _conf;

        static void             handler(int sig);
}; // class Server

}; // namespace Webserv
