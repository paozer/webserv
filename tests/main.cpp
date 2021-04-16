#ifdef NO_UNIT_TESTS

#include "../srcs/Http/Response.hpp"
#include "../srcs/Http/Request.hpp"
#include "../srcs/Http/ChunkedBody.hpp"

#include <iostream>
#include <string>

int main(void)
{
    Webserv::Http::ChunkedBody cb;
    try {
        cb.decode("3\r\nHi \r\n7\r\nhow are\r5\r\n you!\r\n\r\n");
        std::cout <<  "body " << cb.get_body() << std::endl << std::endl;
        std::cout << " trailer part " << cb.get_trailer_part() << std::endl << std::endl;
    } catch (const Webserv::Http::InvalidPacketException& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
