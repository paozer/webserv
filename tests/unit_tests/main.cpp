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
    std::cout << Utils::atoi_base("3e8");
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
