#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Http.hpp"
#include "ChunkedBody.hpp"
#include "../Utils/Logger.hpp"

#include <sstream>

namespace Webserv {
namespace Http {

class Request
{
    public:
        void append (const std::string& packet, size_t client_max_body_size);
        inline bool has_header (const std::string& name) const { return _headers.find(name) != _headers.end(); }
        inline const std::string& get_err_status_code (void) const { return _error_status_code; }
        inline const State& get_state (void) const { return _state; }
        inline const std::string& get_method (void) const { return _request_line[0]; }
        inline const std::string& get_uri (void) const { return _request_line[1]; }
        inline const std::string& get_http_version (void) const { return _request_line[2]; }
        inline const HeaderMap& get_headers (void) const { return _headers; }
        inline const HeaderValues& get_header_values (const std::string& name) const { return _headers.find(name)->second; }
        inline const std::string& get_body (void) const { return _body; }

        // inline void print (std::ostringstream& oss)
        // {
        //     oss << "##############################################################\n";
        //     if (_state == Complete)
        //         oss << "Complete\n";
        //     else if (_state == Incomplete)
        //         oss << "Incomplete\n";
        //     else if (_state == Error)
        //         oss << "Error\n";
        //     else if (_state == Requestline)
        //         oss << "Requestline\n";
        //     else if (_state == Headers)
        //         oss << "Headers\n";
        //     else if (_state == Body)
        //         oss << "Body\n";
        //     else
        //         oss << "woops\n";
        //     oss << "##############################################################\n";
        //     for (auto it = _request_line.begin(); it != _request_line.end(); ++it)
        //         oss << *it + " ";
        //     oss << "\n##############################################################\n";
        //     for (auto it = _headers.begin(); it != _headers.end(); ++it)
        //         oss << it->first << " " << it->second << "\n";
        //     oss << "##############################################################\n";
        //     oss << "[" << _body.substr(0, 60) << "]\n";
        //     oss << "##############################################################\n";
        //     oss << "\n\n";
        // }

    private:
        std::vector<std::string> _request_line;
        HeaderMap _headers;
        std::string _body;
        std::string _packet;
        std::string _error_status_code;
        ChunkedBody _cb;
        State _state;

        /* VALIDITY */
        void evaluate_request_line (void) const;
        void evaluate_headers (void) const;
        void evaluate_body (void) const; // not implemented

        /* PARSING */
        void parse_request_line (void);
        void parse_headers (void);
        HeaderValues parse_header_values (const std::string& field_name, const std::string& field_values) const;
        void parse_body (size_t client_max_body_size);

}; // class HttpRequest

}; // namespace Http
}; // namespace Webserv

#endif
