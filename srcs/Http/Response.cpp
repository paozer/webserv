#include "Response.hpp"

namespace Webserv {
namespace Http {

void Response::build_raw_packet (void)
{
    _raw_packet.clear();
    if (!_status_code.empty())
        _raw_packet = "HTTP/1.1 " + _status_code + " " + status_codes[_status_code] + CRLF;
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

std::map<std::string, std::string> Response::status_codes = Response::create_status_codes_map();

std::map<std::string, std::string> Response::create_status_codes_map (void)
{
    std::map<std::string, std::string> status_codes;
    //Successful 2xx
    status_codes["200"] = "OK";
    status_codes["201"] = "Created";
    status_codes["202"] = "Accepted";
    status_codes["203"] = "Non-Authoritative";
    status_codes["204"] = "No";
    status_codes["205"] = "Reset";
    //Redirection 3xx
    status_codes["300"] = "Multiple";
    status_codes["301"] = "Moved";
    status_codes["302"] = "Found";
    status_codes["303"] = "See";
    status_codes["305"] = "Use";
    status_codes["306"] = "(Unused)";
    status_codes["307"] = "Temporary";
    //Client Error 4xx
    status_codes["400"] = "Bad";
    status_codes["402"] = "Payment";
    status_codes["403"] = "Forbidden";
    status_codes["404"] = "Not";
    status_codes["405"] = "Method";
    status_codes["406"] = "Not";
    status_codes["408"] = "Request";
    status_codes["409"] = "Conflict";
    status_codes["410"] = "Gone";
    status_codes["411"] = "Length";
    status_codes["413"] = "Payload";
    status_codes["414"] = "URI";
    status_codes["415"] = "Unsupported";
    status_codes["417"] = "Expectation";
    status_codes["426"] = "Upgrade";
    //Server Error 5xx
    status_codes["500"] = "Internal";
    status_codes["501"] = "Not";
    status_codes["502"] = "Bad";
    status_codes["503"] = "Service";
    status_codes["504"] = "Gateway";
    status_codes["505"] = "HTTP";
    return status_codes;
}

}; // namespace Http
}; // namespace Webserv
