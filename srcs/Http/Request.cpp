#include "Request.hpp"

namespace Webserv {
namespace Http {

/* PARSING */
std::string Request::parse_raw_packet (std::string packet, size_t max_client_body_size)
{
    clear();
    try {
        evaluate_crlf_usage(packet);
        parse_request_line(packet);
        parse_headers(packet);
        parse_message_body(packet, max_client_body_size);
    } catch (const InvalidPacketException& e) {
        Log::out("Request Parsing", e.what());
        return e.get_err_status_code();
    } catch (const std::exception& e) {
        Log::out("Request Parsing", e.what());
        return "500";
    }
    return "200";
}

void Request::parse_request_line (std::string& raw_packet)
{
    size_t i = raw_packet.find(CRLF);
    if (i == std::string::npos || std::count(raw_packet.begin(), raw_packet.begin() + i, ' ') != 2)
        throw InvalidPacketException("400", "invalid space in request line");
    i = raw_packet.find_first_of(SP, 0);
    _request_line.push_back(raw_packet.substr(0, i));
    size_t j = i + 1;
    i = raw_packet.find_first_of(SP, j);
    _request_line.push_back(raw_packet.substr(j, i - j));
    j = i + 1;
    i = raw_packet.find(CRLF, j);
    _request_line.push_back(raw_packet.substr(j, i - j));
    raw_packet.erase(0, raw_packet.find(CRLF) + 2);
    evaluate_request_line();
}

void Request::parse_headers (std::string& raw_packet)
{
    size_t i = 0;
    size_t j = 0;
    size_t pos_crlf = 0;
    std::string field_name;
    while ((pos_crlf = raw_packet.find(CRLF)) != 0) {
        if (pos_crlf == std::string::npos)
            throw InvalidPacketException("400", "invalid crlf usage");
        if (pos_crlf < (j = raw_packet.find(":")) || j == std::string::npos)
            throw InvalidPacketException("400", "expected new header but got something else");
        field_name = raw_packet.substr(0, j);
        i = raw_packet.find_first_not_of(OWS, j + 1);
        j = raw_packet.find_last_not_of(OWS + CRLF, pos_crlf);
        _headers[field_name] = raw_packet.substr(i, j - i + 1);
        raw_packet.erase(0, pos_crlf + 2);
    }
    evaluate_headers();
}

void Request::parse_message_body (std::string& raw_packet, size_t max_client_body_size)
{
    raw_packet.erase(0, 2);
    if (raw_packet.empty())
        return ;
    if (has_header("Transfer-Encoding")) {
        if (get_header_values("Transfer-Encoding") != "chunked")
            throw InvalidPacketException("501", "unknown transfer encoding");
        ChunkedBody cb;
        cb.decode(raw_packet, max_client_body_size);
        _body = cb.get_body();
        std::string tmp = cb.get_trailer_part();
        parse_headers(tmp);
        _headers.erase("Content-Length");
    } else if (has_header("Content-Length")) {
        int len = Utils::atoi_base(get_header_values("Content-Length"));
        if (len == -1 || static_cast<size_t>(len) != raw_packet.length())
            throw InvalidPacketException("400", "invalid content-length");
        if (static_cast<size_t>(len) > max_client_body_size)
            throw InvalidPacketException("413", "client body size too large");
        _body = raw_packet.substr(0, len);
    } else {
        throw InvalidPacketException("411", "missing transfer-encoding or content-length");
    }
}

/* VALIDITY */
void Request::evaluate_crlf_usage (const std::string& packet) const
{
    if (packet.find(CRLF + CRLF) == std::string::npos)
        throw InvalidPacketException("400", "invalid crlf usage");
    for (size_t i = 0; (i = packet.find_first_of(CRLF, i)) != std::string::npos;) {
        if (packet[i] == '\n') {
            if (i == 0 || packet[i - 1] != '\r')
                throw InvalidPacketException("400", "invalid crlf usage");
            else if (1 < i && packet[i - 2] == '\n')
                return ;
            ++i;
        } else {
            if (i == packet.length() || packet[i + 1] != '\n')
                throw InvalidPacketException("400", "invalid crlf usage");
            else if (i + 2 < packet.length() && packet[i + 2] == '\r')
                return ;
            i += 2;
        }
    }
}

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

/* UTILITIES */
void Request::clear (void)
{
    _request_line.clear();
    _headers.clear();
    _body.clear();
}

}; // namespace Http
}; // namespace Webserv
