#pragma once

#include "Http.hpp"

namespace Webserv {
namespace Http {

class Response
{
    public:
        void build_raw_packet (void);
        inline const std::string& get_raw_packet (void) { return _raw_packet; }

        inline void set_status_code (const std::string& status_code) { _status_code = status_code; }
        inline void set_body (const std::string& body) { _body = body; }
        inline void unset_body (void) { _body.clear(); }
        inline void set_content_length (void) { append_header("Content-Length", Utils::itoa(_body.length())); }
        void append_header (const std::string& field_name, const std::string& field_value);

    private:
        std::string _status_code;
        HeaderMap _headers;
        std::string _body;
        std::string _raw_packet;

        void clear (void);

}; // class Response

}; // namespace Http
}; // namespace Webserv
