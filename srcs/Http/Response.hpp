#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "Http.hpp"
#include "../Configuration/Configuration.hpp"
#include "../Utils/Files.hpp"

namespace Webserv {
namespace Http {

class Response
{
    public:
        void build_raw_packet (void);
        void fill_with_error (const std::string& status_code, const Configuration::server* server);
        inline const std::string& get_raw_packet (void) const { return _raw_packet; }
        inline const std::string& get_status_code (void) const { return _status_code; }
        inline void set_status_code (const std::string& status_code) { _status_code = status_code; }
        inline void set_body (const std::string& body) { _body = body; }
        inline void unset_body (void) { _body.clear(); }
        inline void set_content_length (void) { _headers["Content-Length"] = Utils::itoa(_body.length()); }
        void append_header (const std::string& field_name, const std::string& field_value);

    private:
        std::string _status_code;
        HeaderMap _headers;
        std::string _body;
        std::string _raw_packet;

}; // class Response

}; // namespace Http
}; // namespace Webserv

#endif
