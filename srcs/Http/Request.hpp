#pragma once

#include "Http.hpp"
#include "ChunkedBody.hpp"
#include "../Logger.hpp"

namespace Webserv {
namespace Http {

class Request
{
    public:
        std::string parse_raw_packet (std::string packet, size_t max_client_body_size = 1048576);
        inline bool has_header (const std::string& name) const { return _headers.find(name) != _headers.end(); }

        inline const std::string& get_method (void) const { return _request_line[0]; }
        inline const std::string& get_uri (void) const { return _request_line[1]; }
        inline const std::string& get_http_version (void) const { return _request_line[2]; }
        inline const HeaderMap& get_headers (void) const { return _headers; }
        inline const HeaderValues& get_header_values (const std::string& name) const { return _headers.find(name)->second; }
        inline const std::string& get_body (void) const { return _body; }

    private:
        std::vector<std::string> _request_line;
        HeaderMap _headers;
        std::string _body;

        void clear (void);

        /* VALIDITY */
        void evaluate_crlf_usage (const std::string& packet) const;
        void evaluate_request_line (void) const;
        void evaluate_headers (void) const;
        void evaluate_body (void) const; // not implemented

        /* PARSING */
        void parse_request_line (std::string& raw_packet);
        void parse_headers (std::string& raw_packet);
        HeaderValues parse_header_values (const std::string& field_name, const std::string& field_values) const;
        void parse_message_body (std::string& raw_packet, size_t client_max_body_size);

}; // class HttpRequest

}; // namespace Http
}; // namespace Webserv
