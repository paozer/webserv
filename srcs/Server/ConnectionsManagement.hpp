#ifndef CONNECTIONS_MANAGEMENT
# define CONNECTIONS_MANAGEMENT

#include <iostream>

#include <string>
#include <vector>
#include <map>
#include <list>

#include <sys/select.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

#include "ServerSocket.hpp"
#include "Methods.hpp"
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Logger.hpp"

namespace Webserv {

class ConnectionManagement
{
    public:
        ConnectionManagement();
        ConnectionManagement(std::string const &id, const Configuration &config);


        int     loop_worker();
        int     loop_server(std::vector<ServerSocket> const &serv_sock);
        bool    is_server_fd(int fd, std::vector<ServerSocket> const &serv_sock);
        void    handle_incoming (int fd);
        void    construct_response(int fd);
        void    send_response (int fd);
        void    close_connection (int fd);
        void    close_tmp_file(void);

        int             _max_fd;
        int             _nb_server;
        char            *_buffer;

        int             lost_connections_count;
        struct timeval  _timeout;

        std::string     _id;

        fd_set          _write_fds;
        fd_set          _read_fds;
        fd_set          _tmp_write_fds;
        fd_set          _tmp_read_fds;

        std::map<int, Http::Request>            _incomplete_request;

        private:
            std::string     _s_buffer;
            Configuration   _config;

            struct settings
            {
                settings (void)
                {
                }

                settings (bool close, int off, const std::string& s, int fd)
                    : should_close(close), offset(off), response_queue(s), tmpFileFd(fd), readfile(false)
                {
                }
                bool should_close;
                int offset;
                std::string response_queue;
                int tmpFileFd;
                std::string filename;
                bool readfile;
            };

            std::map<int, struct settings> ready_responses;

            void    send_from_file(struct settings *tmp, int fd);
            void    send_from_queue(struct settings *tmp, int fd);
};

}; // namespace Webserv
#endif
