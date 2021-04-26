#ifndef HTTP_HPP
#define HTTP_HPP

#include "../Utils/Utils.hpp"

#include <ctype.h>

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <exception>

namespace Webserv {
namespace Http {

enum State
{
    Complete,
    Incomplete,
    Error,
    Requestline,
    Headers,
    Body
};

struct case_incensitive_strcmp;
typedef std::string HeaderValues;
typedef std::map<std::string, HeaderValues, case_incensitive_strcmp> HeaderMap;
typedef std::map<std::string, std::string> StatusCodeMap;

struct case_incensitive_strcmp
{
    static inline bool comp (const char& c1, const char& c2)
    {
        return Utils::tolower(c1) < Utils::tolower(c2);
    }

    inline bool operator () (const std::string& s1, const std::string& s2) const
    {
        return lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), comp);
    }
};

class InvalidPacketException : std::exception
{
    public:
        InvalidPacketException (const std::string& err_code, const std::string& err_description)
            : _err_code(err_code), _err_description("HttpPacketException: " + err_description)
        {
        }

        ~InvalidPacketException (void) throw()
        {
        }

        inline const char* what (void) const throw() { return _err_description.c_str(); }
        inline const char* get_err_status_code (void) const { return _err_code.c_str(); }

    private:
        std::string _err_code;
        std::string _err_description;

}; // class InvalidPacketException

static const char* ARR_STD_METHODS[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
static const size_t NB_STD_METHODS = sizeof(ARR_STD_METHODS) / sizeof(char*);
static const std::vector<std::string> STANDARD_METHODS (ARR_STD_METHODS, ARR_STD_METHODS + NB_STD_METHODS);

static StatusCodeMap create_status_codes_map(void);
static StatusCodeMap STATUS_CODES = create_status_codes_map();
static const std::string SP = " ";
static const std::string OWS = " \t";
static const std::string CRLF = "\r\n";
static const bool TCHARS[] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, true, true, true, true, true, false, false, true, true, false, true, true, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, true, false, true, false, false};

inline StatusCodeMap create_status_codes_map (void)
{
    std::map<std::string, std::string> status_codes;
    //Successful 2xx
    status_codes["200"] = "OK";
    status_codes["201"] = "Created";
    status_codes["202"] = "Accepted";
    status_codes["203"] = "Non-Authoritative Information";
    status_codes["204"] = "No Content";
    status_codes["205"] = "Reset Content";
    status_codes["206"] = "Partial Content";
    //Redirection 3xx
    status_codes["300"] = "Multiple Choice";
    status_codes["301"] = "Moved Permanently";
    status_codes["302"] = "Found";
    status_codes["303"] = "See Other";
    status_codes["304"] = "Not Modified";
    status_codes["307"] = "Temporary Redirect";
    status_codes["308"] = "Permanent Redirect";
    //Client Error 4xx
    status_codes["400"] = "Bad Request";
    status_codes["401"] = "Unauthorized";
    status_codes["403"] = "Forbidden";
    status_codes["404"] = "Not Found";
    status_codes["405"] = "Method Not Allowed";
    status_codes["406"] = "Not Acceptable";
    status_codes["407"] = "Proxy Authentication Required";
    status_codes["408"] = "Request Timeout";
    status_codes["409"] = "Conflict";
    status_codes["410"] = "Gone";
    status_codes["411"] = "Length Required";
    status_codes["412"] = "Precondition Failed";
    status_codes["413"] = "Payload Too Large";
    status_codes["414"] = "URI Too Long";
    status_codes["415"] = "Unsupported Media Type";
    status_codes["416"] = "Range Not Satisfiable";
    status_codes["417"] = "Expectation Failed";
    status_codes["418"] = "I'm a teapot";
    status_codes["421"] = "Misdirected Request";
    status_codes["426"] = "Upgrade Required";
    status_codes["428"] = "Precondition Required";
    status_codes["429"] = "Too Many Requests";
    status_codes["431"] = "Request Header Fields Too Large";
    status_codes["451"] = "Unavailable For Legal Reasons";
    //Server Error 5xx
    status_codes["500"] = "Internal Server Error";
    status_codes["501"] = "Not Implemented";
    status_codes["502"] = "Bad Gateway";
    status_codes["503"] = "Service Unavailable";
    status_codes["504"] = "Gateway Timeout";
    status_codes["505"] = "HTTP Version Not Supported";
    status_codes["506"] = "Variant Not Supported";
    status_codes["510"] = "Not Extended";
    status_codes["511"] = "Network Authentication Required";
    return status_codes;
}

}; // namespace Http
}; // namespace Webserv

#endif
