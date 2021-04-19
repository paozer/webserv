#ifdef NO_UNIT_TESTS

#include "../../srcs/Http/Request.hpp"
#include "../../srcs/Http/Response.hpp"
#include "../../srcs/Methods.hpp"
#include "../../srcs/Routing.hpp"

#include <iostream>
#include <string>

using namespace Webserv;

int main(void)
{
    Configuration config ("tests/unit_tests/conf/config.conf");

    Http::Request request;
    std::cout << "request parsing returned " << request.parse_raw_packet("PUT /put/somefile HTTP/1.1\r\nHost: webserv\r\n\r\n") << "\n";

    Http::Response response = Methods::method_handler(request, config);
    response.build_raw_packet();
    std::cout << response.get_raw_packet() << std::endl;
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
