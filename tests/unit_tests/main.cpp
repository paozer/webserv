#ifdef NO_UNIT_TESTS

#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include "../../srcs/Utils/Time.hpp"
#include "../../srcs/Utils/Files.hpp"

int main(void)
{
    std::string s;
    Webserv::Files::fill_with_file_content (s, "./tests/intra_tests/passwd");
    std::list<std::string> l = Webserv::Utils::split(s, "\n");
    for (auto it = l.begin(); it != l.end(); ++it)
        std::cout << *it << std::endl;
    return 0;
}

#else

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#endif
