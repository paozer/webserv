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

namespace Webserv {

/*
**  MODIFY allowed_methods[] and allowed_line_start[]
**  to accept new configuration 
*/

static std::string allowed_methods[] = { 
    "GET",      "PUT",      "HEAD", 
    "POST",     "DELETE",   "CONNECT",
    "OPTIONS", "TRACE",     "" 
};

static std::string allowed_line_start[] = {
    "{",                    "}",                    "#",
    "server",               "listen",               "location", 
    "root",                 "index",                "client_max_body_size",
    "autoindex",            "cgi_extension",        "cgi_path", 
    "method",               "server_name",          "error_page",           
    "upload_path",          "upload_enable",        ""
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

std::vector<std::list<std::string> >    read_file(std::string file);

void                        undo_whitespace(std::string &line);
void                        count_and_replace_sometimes(std::string &line);

};

#endif