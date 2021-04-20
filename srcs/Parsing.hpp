#ifndef PARSING_HPP
# define PARSING_HPP

# define BUFFER_SIZE 1024

# include <list>
# include <vector>
# include <iostream>
# include <unistd.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sstream>

# include "Utils.hpp"
# include "Config.hpp"

namespace Webserv {
namespace Parsing {
/*
**  MODIFY allowed_methods[] and allowed_line_start[]
**  to accept new configuration in block
**  Modify allowed_line_out[]
**  to accept new block declaration
*/

static std::string allowed_methods[] = {
    "GET",      "PUT",      "HEAD",
    "POST",     "DELETE",   "CONNECT",
    "OPTIONS", "TRACE",     ""
};

static std::string allowed_line_start[] = {
    "{",                    "}",                    "#",
    "\n",                   "listen",               "location",
    "root",                 "index",                "client_max_body_size",
    "autoindex",            "cgi_extension",        "cgi_path",
    "method",               "server_name",          "error_page",
    "upload_path",          "upload_enable",        "server",
    "nb_workers",           "print_configuration",  "workers_max_connections",
    "log_enabled",
    ""
};

static std::string allowed_line_out[] = {
    "server",               "log_enabled",          "#",
    "nb_workers",           "print_configuration",  "workers_max_connections",
    "\n",                   ""
};

class ParsingException : public std::exception
{
    public:
        ParsingException(int line = 0, std::string msg = "Cannot parse the actual config file.")
            : _msg("Err: " + (line != 0 ? "line:" + Utils::itoa(line) + " " + msg : msg)) {};
        ~ParsingException() throw() {};
        const char *what() const throw()
        { return _msg.c_str(); };
    private:
        std::string _msg;
};

std::vector<std::list<std::string> >    read_file(std::string const &file);
void                                    undo_whitespace(std::string &line);
void                                    count_and_replace_sometimes(std::string &line);
void                                    parse_workers(std::string &line, int nb_line);
bool                                    parse_line_out_of_blocks(std::list<std::string> &conf, std::string &line, int nb_line);

};
};

#endif