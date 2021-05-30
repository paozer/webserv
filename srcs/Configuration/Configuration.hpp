#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

class Configuration {};

#include "Parsing.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/Files.hpp"

#include <map>
#include <vector>
#include <string>

namespace Webserv {

static std::string allowed_location[] = {
   "root",                      "cgi_extension",
   "cgi_path",                  "index",
   "method",                    "autoindex",
   "client_max_body_size",      "upload_enable",
   "auth_basic",                "auth_basic_user_file",
   "upload_path",
   ""
};

class Configuration
{
    private:
        typedef std::pair<std::string, std::string> pair;
        typedef std::map<std::string, std::string> smap;
        typedef std::vector<std::list<std::string> > lvector;

    public:
        struct location;

        struct server
        {
            int                             _id;
            std::string                     _server_name;
            std::string                     _root;
            std::pair<int, std::string>     _listen;
            std::map<int, std::string>      _error_pages;
            std::vector<struct location>    _locations;
            std::string                     auth;
            std::string                     auth_path;
            //std::list<std::string>          auth_credentials;
        };

        struct location
        {
            bool                            _autoindex;
            bool                            _upload_enable;
            long                            _client_max_body_size;
            std::string                     _name;
            std::string                     _root;
            std::string                     _index;
            std::string                     _upload_path;
            std::string                     _cgi_extension;
            std::vector<std::string>        _cgi_path;
            std::vector<std::string>        _method;
            bool                            _set_auto_index;
            std::string                     auth;
            std::string                     auth_path;
            std::list<std::string>          auth_credentials;
        };

        struct default_conf
        {
            long                        client_max_body_size;
            bool                        auto_idx;
            std::map<int, std::string>  error_pages;
            std::string                 loc_name;
        };

        int             max_workers;
        int             max_connections_workers;
        bool            print_conf;
        bool            log;
        default_conf    def_conf;
        std::string     auth;
        std::string     auth_path;

    public:
        void                            parse(std::string const &file);
        inline size_t                   get_nb_server() const { return _servers.size(); }
        void                            print_configuration();
        struct server*                  get_server(int id);
        inline const std::vector<struct server>& get_servers (void) const { return _servers; }

    private:
        void                            check_port(void);
        void                            set_default(std::list<std::string> &conf);
        void                            complete_config(Configuration::server &serv);
        void                            complete_location(Configuration::location &loc, Configuration::server &server);
        void                            load_config(std::list<std::string> &conf);
        struct location                 load_location(std::list<std::string>::iterator &it);

        std::vector<struct server>      _servers;
        std::string                     _file;
};


class ConfException : public std::exception
{
    public:
        ConfException(const std::string& line, const std::string& msg)
            : _msg("Err: " + line + " " + msg) { }
        ~ConfException() throw() { }
        const char *what() const throw() { return _msg.c_str(); }

    private:
        std::string _msg;

};

};
#endif
