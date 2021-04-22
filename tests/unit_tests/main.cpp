#ifdef NO_UNIT_TESTS

#include "../../srcs/Http/Request.hpp"
#include "../../srcs/Http/Response.hpp"
#include "../../srcs/Server/Methods.hpp"
#include "../../srcs/Server/Routing.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace Webserv;

int main(void)
{
    Http::Request request;
    request.append("PUT /index.html HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nHello\r\n");
    std::ostringstream oss;
    request.print_state(oss);
    std::cout << oss.str();
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
