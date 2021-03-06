#include "Request.hpp"

namespace Webserv {
namespace Http {

/* PARSING */
void Request::append (const std::string& packet)
{
    _packet.append(packet);
    try {
        if (_request_line.empty()) {
            _state = Requestline;
            parse_request_line();
        }
        if (_state == Headers) {
            parse_headers();
        }
        if (_state == Body) {
            parse_body();
        }
        if (_state == Complete) {
            evaluate_request_line();
            evaluate_headers();
        }
    } catch (const InvalidPacketException& e) {
        _state = Error;
        _error_status_code = e.get_err_status_code();
        Log::out("Request Parsing", e.what());
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
    _state = Headers;
}

void Request::parse_headers (void)
{
    size_t i = 0;
    size_t j = 0;
    size_t pos_crlf = 0;
    std::string field_name;
    while ((pos_crlf = _packet.find(CRLF)) != 0 && pos_crlf != std::string::npos) {
        if (pos_crlf < (j = _packet.find(":")) || j == std::string::npos)
            throw InvalidPacketException("400", "expected new header but got something else");
        if (pos_crlf != _packet.find_first_of(CRLF))
            throw InvalidPacketException("400", "invalid crlf usage");
        field_name = _packet.substr(0, j);
        i = _packet.find_first_not_of(OWS, j + 1);
        j = _packet.find_last_not_of(OWS + CRLF, pos_crlf);
        _headers[field_name] = _packet.substr(i, j - i + 1);
        _packet.erase(0, pos_crlf + 2);
    }
    if (pos_crlf == 0) {
        _state = Body;
        _packet.erase(0, 2);
    }
}

void Request::parse_body (void)
{
    if (has_header("Transfer-Encoding")) {
        if (get_header_values("Transfer-Encoding") != "chunked")
            throw InvalidPacketException("501", "unknown transfer encoding");
        _cb.decode(_packet);
        if (_cb.get_state() == Complete) {
            _body = _cb.get_body();
            _state = Complete;
            _headers.erase("Content-Length");
        } else if (_cb.get_state() == Error) {
            throw InvalidPacketException("400", "invalid chunked message");
        }
    } else if (has_header("Content-Length")) {
        int len = Utils::atoi_base(get_header_values("Content-Length"), "0123456789");
        if (len == -1)
            throw InvalidPacketException("400", "invalid content-length");
        _body += _packet;
        _packet.clear();
        if (_body.length() == static_cast<size_t>(len))
            _state = Complete;
        else if (_body.length() > static_cast<size_t>(len))
            _state = Error;
    } else if (!_packet.empty()){
        throw InvalidPacketException("411", "missing transfer-encoding or content-length");
    } else {
        _state = Complete;
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
        if (it->first.find_first_not_of(TCHARS) != std::string::npos)
            throw InvalidPacketException("400", "invalid character in header field-name");
        // if (it->second.find_not_of(TCHARS) != std::string::npos)
        //     throw InvalidPacketException("400", "invalid character in header field-value");
    }
    // if this check is removed here add it to somwhere along the way
    if (!has_header("Host"))
        throw InvalidPacketException("400", "missing host header");
}

}; // namespace Http
}; // namespace Webserv
