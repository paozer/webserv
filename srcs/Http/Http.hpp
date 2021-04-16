#pragma once

#include "../Utils.hpp"

#include <cctype>

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <exception>

namespace Webserv {
namespace Http {

// TODO
// - handle crlf usage validity
// - handle to long lines
// - handle header duplicates
// - make header values strings not vector of string (?)

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

        const char* what (void) const throw() { return _err_description.c_str(); }
        const char* get_err_status_code (void) const { return _err_code.c_str(); }

    private:
        std::string _err_code;
        std::string _err_description;

}; // class InvalidPacketException

static const char* ARR_STD_METHODS[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
static const size_t NB_STD_METHODS = sizeof(ARR_STD_METHODS) / sizeof(char*);
static const std::vector<std::string> STANDARD_METHODS (ARR_STD_METHODS, ARR_STD_METHODS + NB_STD_METHODS);

static const std::string SP = " ";
static const std::string OWS = " \t";
static const std::string CRLF = "\r\n";
static const bool TCHARS[] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, true, true, true, true, true, false, false, true, true, false, true, true, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, true, false, true, false, false};

}; // namespace Http
}; // namespace Webserv
