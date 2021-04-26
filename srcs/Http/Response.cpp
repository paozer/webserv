#include "Response.hpp"

namespace Webserv {
namespace Http {

void Response::fill_with_error (const std::string& status_code, const Configuration::server* server)
{
    _status_code = status_code;
    if (server != NULL) {
        std::map<int, std::string>::const_iterator it = server->_error_pages.find(Utils::atoi(status_code));
        if (it != server->_error_pages.end())
            _body = Utils::get_file_content(it->second);
    }
}

void Response::build_raw_packet (void)
{
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

}; // namespace Http
}; // namespace Webserv
