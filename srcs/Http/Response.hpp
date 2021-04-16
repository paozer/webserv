#include "Http.hpp"

namespace Webserv {
namespace Http {

class Response
{
    public:
        void clear (void);
        void build_raw_packet (void);
        inline const std::string& get_raw_packet (void) { return _raw_packet; }

        inline void set_status_code (const std::string& status_code) { _status_code = status_code; }
        inline void set_body (const std::string& body) { _body = body; }
        void append_header (const std::string& field_name, const std::string& field_value);

    private:
        std::string _status_code;
        HeaderMap _headers;
        std::string _body;
        std::string _raw_packet;

        static StatusCodeMap create_status_codes_map(void);
        static StatusCodeMap status_codes;

}; // class Response

}; // namespace Http
}; // namespace Webserv
