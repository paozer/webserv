#include "../../srcs/Methods.hpp"
#include "../../srcs/Config.hpp"

#include "catch.hpp"

TEST_CASE("selecting server/locations based on requests parts", "[Methods]")
{
    Webserv::Configuration conf ("tests/unit_tests/conf/configpao.conf");
    const Webserv::Configuration::server* s;
    const Webserv::Configuration::location* l;
    std::vector<std::string> methods;

    SECTION("valid servers and locations") {
        SECTION("") { s = Webserv::Routing::select_server(conf, "127.0.0.1", "8080", "webserv"); }
        SECTION("") { s = Webserv::Routing::select_server(conf, "127.0.0.1", "8080", "webserv.com"); }
        SECTION("") { s = Webserv::Routing::select_server(conf, "127.0.0.1", "8080", ""); }

        REQUIRE( s != NULL );
        CHECK( s->_server_name == "webserv" );

        l = Webserv::Routing::select_location(s, "/");
        REQUIRE( l != NULL );
        methods = { "PUT" };
        CHECK( l->_method == methods );
        l = Webserv::Routing::select_location(s, "/somefile");
        REQUIRE( l != NULL );
        CHECK( l->_method == methods );

        l = Webserv::Routing::select_location(s, "/php");
        REQUIRE( l != NULL );
        methods = { "DELETE", "PUT", "OPTIONS" };
        CHECK( l->_method == methods );
        l = Webserv::Routing::select_location(s, "/php/something/somemore");
        REQUIRE( l != NULL );
        CHECK( l->_method == methods );
        l = Webserv::Routing::select_location(s, "/phpppppppppppp");
        REQUIRE( l != NULL );
        CHECK( l->_method == methods );

        l = Webserv::Routing::select_location(s, "/wordpress");
        REQUIRE( l != NULL );
        methods = { "HEAD", "POST" };
        CHECK( l->_method == methods );
        l = Webserv::Routing::select_location(s, "/wordpresss");
        REQUIRE( l != NULL );
        CHECK( l->_method == methods );
    }
    SECTION("default constructed server block") {
        s = Webserv::Routing::select_server(conf, "42.42.42.42", "80", "webserv");
        REQUIRE( s != NULL );
        l = Webserv::Routing::select_location(s, "/");
        REQUIRE( l != NULL );
        l = Webserv::Routing::select_location(s, "/wordpress");
        REQUIRE( l != NULL );
    }
    SECTION("invalid locations") {
        SECTION("") { s = Webserv::Routing::select_server(conf, "42.42.42.42", "80", "webserv"); }
        SECTION("") { s = Webserv::Routing::select_server(conf, "127.0.0.1", "8080", "webserv"); }

        l = Webserv::Routing::select_location(s, "0");
        REQUIRE( l == NULL );
        l = Webserv::Routing::select_location(s, "someweirdstuff");
        REQUIRE( l == NULL );
        l = Webserv::Routing::select_location(s, "");
        REQUIRE( l == NULL );
    }
    SECTION("invalid servers") {
        s = Webserv::Routing::select_server(conf, "42.42.42.42", "8080", "webserv");
        REQUIRE( s == NULL );
        s = Webserv::Routing::select_server(conf, "42.42.42.422", "", "webserv");
        REQUIRE( s == NULL );
        s = Webserv::Routing::select_server(conf, "ipaddressthatisnotanipaddress", "80", "someserver");
        REQUIRE( s == NULL );
    }
}
