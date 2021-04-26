#ifdef NO_UNIT_TESTS

#include "../../srcs/Http/Request.hpp"
#include "../../srcs/Http/Response.hpp"
#include "../../srcs/Server/Methods.hpp"
#include "../../srcs/Server/Routing.hpp"
#include "../../srcs/Http/Authentication.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace Webserv;

int main(void)
{
    std::string s = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
    std::cout << Http::base64_decode(s);
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
