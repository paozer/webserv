#include "Request.hpp"
#include "ChunkedBody.hpp"

namespace Webserv {
namespace Http {

/* PARSING */
void Request::parse_raw_packet (std::string packet, size_t max_client_body_size)
{
    clear();
    evaluate_crlf_usage(packet);
    parse_first_line(packet);
    parse_headers(packet);
    parse_message_body(packet, max_client_body_size);
    evaluate_validity();
}

void Request::parse_first_line (std::string& raw_packet)
{
    size_t i = raw_packet.find(CRLF);
    if (i == std::string::npos || std::count(raw_packet.begin(), raw_packet.begin() + i, ' ') != 2)
        throw InvalidPacketException("400", "invalid amounts of spaces in first line");
    i = raw_packet.find_first_of(SP, 0);
    _request_line.push_back(raw_packet.substr(0, i));
    size_t j = i + 1;
    i = raw_packet.find_first_of(SP, j);
    _request_line.push_back(raw_packet.substr(j, i - j));
    j = i + 1;
    i = raw_packet.find(CRLF, j);
    _request_line.push_back(raw_packet.substr(j, i - j));
    raw_packet.erase(0, raw_packet.find(CRLF) + 2);
}

void Request::parse_headers (std::string& raw_packet)
{
    size_t i = 0;
    size_t j = 0;
    size_t pos_crlf = 0;
    std::string field_name;
    while ((pos_crlf = raw_packet.find(CRLF)) != 0) {
        if (pos_crlf == std::string::npos)
            throw InvalidPacketException("400", "expected empty line but got something else");
        if (pos_crlf < (j = raw_packet.find(":")) || j == std::string::npos)
            throw InvalidPacketException("400", "expected header but got something else " + raw_packet);
        field_name = raw_packet.substr(0, j);
        i = raw_packet.find_first_not_of(OWS, j + 1);
        j = raw_packet.find_last_not_of(OWS + CRLF, pos_crlf);
        _headers[field_name] = raw_packet.substr(i, j - i + 1);
        raw_packet.erase(0, pos_crlf + 2);
    }
}

void Request::parse_message_body (std::string& raw_packet, size_t max_client_body_size)
{
    raw_packet.erase(0, 2);
    if (raw_packet.empty())
        return ;
    if (has_header("Transfer-Encoding")) {
        if (get_header_values("Transfer-Encoding") != "chunked")
            throw InvalidPacketException("501", "server only understands chunked encoding");
        ChunkedBody cb;
        cb.decode(raw_packet, max_client_body_size);
        _body = cb.get_body();
        std::string tmp = cb.get_trailer_part();
        parse_headers(tmp);
        _headers.erase("Content-Length");
    } else if (has_header("Content-Length")) {
        int len = Utils::atoi_base(get_header_values("Content-Length"));
        if (len == -1 || static_cast<size_t>(len) > raw_packet.length())
            throw InvalidPacketException("400", "invalid content-length");
        if (static_cast<size_t>(len) > max_client_body_size)
            throw InvalidPacketException("413", "client body size too large");
        _body = raw_packet.substr(0, len);
        if (_body.length() != static_cast<size_t>(len))
            throw InvalidPacketException("400", "body length does not match content-length");
    } else {
        throw InvalidPacketException("400", "body was not indicated by headers");
    }
}

/* VALIDITY */
void Request::evaluate_crlf_usage (const std::string& packet) const
{
    if (packet.find(CRLF + CRLF) == std::string::npos)
        throw InvalidPacketException("400", "invalid end of line");
    for (size_t i = 0; (i = packet.find_first_of(CRLF, i)) != std::string::npos;) {
        if (packet[i] == '\n') {
            if (i == 0 || packet[i - 1] != '\r')
                throw InvalidPacketException("400", "invalid end of line");
            else if (1 < i && packet[i - 2] == '\n')
                return ;
            ++i;
        } else {
            if (i == packet.length() || packet[i + 1] != '\n')
                throw InvalidPacketException("400", "invalid end of line");
            else if (i + 2 < packet.length() && packet[i + 2] == '\r')
                return ;
            i += 2;
        }
    }
}
void Request::evaluate_validity (void) const
{
    evaluate_request_line();
    evaluate_headers();
    //evaluate_body();
}

void Request::evaluate_request_line (void) const
{
    if (std::find(STANDARD_METHODS.begin(), STANDARD_METHODS.end(), get_http_method()) == STANDARD_METHODS.end())
        throw InvalidPacketException("400", "not a standard http method");
    if (get_http_version().compare("HTTP/1.1") != 0)
        throw InvalidPacketException("505", "invalid http-version");
}

void Request::evaluate_headers (void) const
{
    for (HeaderMap::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        if (it->first.empty())
            throw InvalidPacketException("400", "empty header name");
        for (size_t i = 0; i < it->first.length(); ++i) {
            if (!TCHARS[static_cast<size_t>(it->first[i])])
                throw InvalidPacketException("400", "invalid character in header name: " + it->first);
        }
    }
}

/* UTILITIES */
void Request::clear (void)
{
    _request_line.clear();
    _headers.clear();
    _body.clear();
}

}; // namespace Http
}; // namespace Webserv
