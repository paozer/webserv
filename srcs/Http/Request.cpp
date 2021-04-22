#include "Request.hpp"

namespace Webserv {
namespace Http {

/* PARSING */
void Request::append (const std::string& packet)
{
    try {
        _packet.append(packet);
        if (_request_line.size() != 3) {
            _state = Requestline;
            parse_request_line();
            if (_request_line.size() != 3)
                return ;
            evaluate_request_line();
            _state = Headers;
        }
        if (_state == Headers) {
            parse_headers();
            evaluate_headers();
            if (_packet.find(CRLF) == 0) {
                _state = Body;
                _packet.length() == 2 ? _state = Complete : 0;
            }
        }
        if (_state == Body) {
            parse_message_body();
        }
    } catch (const InvalidPacketException& e) {
        Log::out("Request Parsing", e.what());
        _error_status_code = e.what();
        _state = Error;
    } catch (const std::exception& e) {
        Log::out("Request Parsing", e.what());
        _error_status_code = "500";
        _state = Error;
    }
}

void Request::parse_request_line (void)
{
    size_t i = _packet.find(CRLF);
    if (i == std::string::npos)
        return ;
    if (std::count(_packet.begin(), _packet.begin() + i, ' ') != 2)
        throw InvalidPacketException("400", "invalid space in request line");
    i = _packet.find_first_of(SP, 0);
    _request_line.push_back(_packet.substr(0, i));
    size_t j = i + 1;
    i = _packet.find_first_of(SP, j);
    _request_line.push_back(_packet.substr(j, i - j));
    j = i + 1;
    i = _packet.find(CRLF, j);
    _request_line.push_back(_packet.substr(j, i - j));
    _packet.erase(0, _packet.find(CRLF) + 2);
}

void Request::parse_headers (void)
{
    size_t i = 0;
    size_t j = 0;
    size_t pos_crlf = 0;
    std::string field_name;
    while ((pos_crlf = _packet.find(CRLF)) != 0) {
        if (pos_crlf == std::string::npos)
            return ;
        if (pos_crlf < (j = _packet.find(":")) || j == std::string::npos)
            throw InvalidPacketException("400", "expected new header but got something else");
        field_name = _packet.substr(0, j);
        i = _packet.find_first_not_of(OWS, j + 1);
        j = _packet.find_last_not_of(OWS + CRLF, pos_crlf);
        _headers[field_name] = _packet.substr(i, j - i + 1);
        _packet.erase(0, pos_crlf + 2);
    }
}

void Request::parse_message_body (size_t max_client_body_size)
{
    _packet.erase(0, 2);
    if (_packet.empty())
        return ;
    if (has_header("Transfer-Encoding")) {
        if (get_header_values("Transfer-Encoding") != "chunked")
            throw InvalidPacketException("501", "unknown transfer encoding");
        ChunkedBody cb;
        if (cb.decode(_packet, max_client_body_size) == Complete)
            _state = Complete;
        _body = cb.get_body();
        _packet = cb.get_trailer_part();
        parse_headers();
        _headers.erase("Content-Length");
        evaluate_headers();
    } else if (has_header("Content-Length")) {
        int len = Utils::atoi_base(get_header_values("Content-Length"));
        if (len == -1)
            throw InvalidPacketException("400", "invalid content-length");
        if (static_cast<size_t>(len) > max_client_body_size)
            throw InvalidPacketException("413", "client body size too large");
        _body = _packet.substr(0, len);
        if (static_cast<size_t>(len) == _packet.length())
            _state = Complete;
    } else {
        throw InvalidPacketException("411", "missing transfer-encoding or content-length");
    }
}

/* VALIDITY */
void Request::evaluate_request_line (void) const
{
    if (std::find(STANDARD_METHODS.begin(), STANDARD_METHODS.end(), get_method()) == STANDARD_METHODS.end())
        throw InvalidPacketException("400", "invalid request method");
    if (get_http_version() != "HTTP/1.1")
        throw InvalidPacketException("505", "invalid http-version");
}

void Request::evaluate_headers (void) const
{
    for (HeaderMap::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        if (it->first.empty())
            throw InvalidPacketException("400", "empty header field-name");
        for (size_t i = 0; i < it->first.length(); ++i) {
            if (!TCHARS[static_cast<size_t>(it->first[i])])
                throw InvalidPacketException("400", "invalid character in header field-name");
        }
    }
}

}; // namespace Http
}; // namespace Webserv
