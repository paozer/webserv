#include "Response.hpp"

namespace Webserv {
namespace Http {

void Response::build_raw_packet (void)
{
    _raw_packet.clear();
    if (!_status_code.empty())
        _raw_packet = "HTTP/1.1 " + _status_code + " " + STATUS_CODES[_status_code] + CRLF;
    for (HeaderMap::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        if (!it->second.empty())
            _raw_packet += it->first + ": " + it->second + CRLF;
    }
    _raw_packet += CRLF + _body;
}

void Response::append_header (const std::string& field_name, const std::string& field_value)
{
    if (_headers.find(field_name) != _headers.end())
        _headers[field_name].append(", " + field_value);
    else
        _headers[field_name] = field_value;
}

void Response::clear (void)
{
    _status_code.clear();
    _headers.clear();
    _body.clear();
}

}; // namespace Http
}; // namespace Webserv
