#include "Response.hpp"

namespace Webserv {
namespace Http {

Http::Response Response::create_standard_response (void)
{
    Http::Response response;
    response.set_status_code("200");
    response.append_header("Date", Time::get_http_formatted_now());
    response.append_header("Server", "webserv/1.0");
    return response;
}

void Response::fill_with_error (const std::string& status_code, const Configuration::server* server)
{
    _body.clear();
    _status_code = status_code;
    bool config_error_page_was_set = false;
    if (server != NULL) {
        std::map<int, std::string>::const_iterator it = server->_error_pages.find(Utils::atoi(status_code));
        if (it != server->_error_pages.end()
                && Files::fill_with_file_content(_body, it->second) == 0) {
            config_error_page_was_set = true;
            _headers["Content-Type"] = get_media_type(it->second);
        }
    }
    if (!config_error_page_was_set) {
        std::string text = status_code;;
        if (STATUS_CODES.find(status_code) != STATUS_CODES.end())
            text += ": " + STATUS_CODES[status_code];
        _body = "<center><h1>Error " + text + "</h1></center>";
        _headers["Content-Type"] = "text/html";
    }
    if (status_code == "503")
        _headers["Retry-After"] = "120";
    _headers["Connnection"] = "close";
}

void Response::append_header (const std::string& field_name, const std::string& field_value)
{
    if (_headers.find(field_name) != _headers.end())
        _headers[field_name].append(", " + field_value);
    else
        _headers[field_name] = field_value;
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

bool Response::should_close (void) const
{
    HeaderMap::const_iterator it = _headers.find("Connection");
    if (it == _headers.end())
        return false;
    return it->second == "close";
}

}; // namespace Http
}; // namespace Webserv
