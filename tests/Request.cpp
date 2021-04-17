#include "catch.hpp"
#include "../srcs/Http/Request.hpp"

//TODO check if trailer part contains illegal headers

/* TESTING PARSING OF VALID REQUESTS */
TEST_CASE("valid request lines", "[Http][Request]")
{
    Webserv::Http::Request r;
    SECTION("all standard request methods are recognized") {
        std::vector<std::string> methods = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS" };
        for (auto it = methods.begin(); it != methods.end(); ++it) {
            CHECK( r.parse_raw_packet(*it + " /index.html HTTP/1.1\r\n\r\n") == "200" );
            CHECK( r.get_http_method() == *it );
        }
    }
    SECTION("URI are parsed correctly") {
        CHECK( r.parse_raw_packet("PUT /something HTTP/1.1\r\n\r\n") == "200" );
        CHECK( r.get_uri() == "/something" );
        CHECK( r.parse_raw_packet("GET / HTTP/1.1\r\n\r\n") == "200" );
        CHECK( r.get_uri() == "/" );
        CHECK( r.parse_raw_packet("DELETE http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1\r\n\r\n") == "200" );
        CHECK( r.get_uri() == "http://www.w3.org/pub/WWW/TheProject.html" );
    }
}

TEST_CASE("valid headers", "[Http][Request]")
{
    Webserv::Http::Request r;
    std::vector<Webserv::Http::HeaderValues> values = { {"$va1lue"}, {"2value!"}, {"~value3"}, {"_something.*, 012345abc#"} };
    REQUIRE( r.parse_raw_packet("PUT /something HTTP/1.1\r\nHeader: $va1lue   \r\n2HeadeR:   2value!\r\nThIrdHeaDer:~value3\r\nFOURTHHEADER:    _something.*, 012345abc# \r\n\r\n") == "200" );
    REQUIRE( r.has_header("Header") );
    CHECK( r.get_header_values("Header") == values[0] );
    REQUIRE( r.has_header("2header") );
    CHECK( r.get_header_values("2header") == values[1] );
    REQUIRE( r.has_header("Thirdheader") );
    CHECK( r.get_header_values("Thirdheader") == values[2] );
    REQUIRE( r.has_header("Fourthheader") );
    CHECK( r.get_header_values("Fourthheader") == values[3] );
}

TEST_CASE("valid body", "[Http][Request]")
{
    Webserv::Http::Request r;
    SECTION("simple message body is parsed correctly") {
        REQUIRE( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nContent-Length:  12\r\n\r\nHello World!") == "200" );
        CHECK( r.get_body() == "Hello World!");
        REQUIRE( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nContent-Length:  12\r\n\r\nHello\nWorld!") == "200" );
        CHECK( r.get_body() == "Hello\nWorld!");
        REQUIRE( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nContent-Length:  12\r\n\r\nHello\rWorld!") == "200" );
        CHECK( r.get_body() == "Hello\rWorld!");
    }
    SECTION("content-length header is removed when is body chunked") {
        REQUIRE( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nContent-Length:  10 \r\nTransfer-Encoding: chunked   \r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n000;ext=value\r\n\r\n") == "200" );
        CHECK_FALSE( r.has_header("Content-Length") );
        CHECK( r.has_header("Transfer-Encoding") );
        CHECK( r.get_body() == "Hello!" );
    }
    SECTION("headers in trailer part are parsed correctly") {
        REQUIRE( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n000;ext=value\r\ntrailer-header: somevalue \r\n\r\n") == "200" );
        REQUIRE( r.has_header("Trailer-Header") );
        CHECK( r.get_header_values("Trailer-Header") == "somevalue" );
        REQUIRE( r.parse_raw_packet("PUT /somefile HTTP/1.1\r\nTransfer-Encoding:   chunked  \r\n\r\n6\r\n#!some\r\n12\r\n executable \r\n6\r\nscript\r\n000;ext1=value1;ext2=value2;ext3\r\ntrailer-header: somevalue \r\nother-trailer-header:someothervalue\r\nsomemoreheader: values          \r\n\r\n") == "200" );
        CHECK( r.get_body() == "#!some executable script" );
        REQUIRE( r.has_header("Trailer-Header") );
        CHECK( r.get_header_values("Trailer-Header") == "somevalue" );
        REQUIRE( r.has_header("Other-Trailer-Header") );
        CHECK( r.get_header_values("Other-Trailer-Header") == "someothervalue" );
        REQUIRE( r.has_header("SOMEMOREHEADER") );
        CHECK( r.get_header_values("SOMEMOREHEADER") == "values" );
    }
}

/* TESTING DETECTION OF INVALID REQUESTS */
TEST_CASE("invalid request lines", "[Http][Request]")
{
    Webserv::Http::Request r;
    SECTION("single spaces separate request line parameters") {
        CHECK( r.parse_raw_packet("GET  /index.html HTTP/1.1\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("GET     /index.html HTTP/1.1\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("PUT /index.html  HTTP/1.1\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.1 \r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("HEAD             /somewhere          HTTP/1.1\r\n\r\n") == "400" );
    }
    SECTION("three parameters compose a request line") {
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.1 maliciousstuff\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.1 maliciousstuff somemore\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("GET HTTP/1.1 \r\n\r\n") == "505" );
        CHECK( r.parse_raw_packet("GET HTTP/1.1\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("GET \r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("\r\n\r\n") == "400" );
    }
    SECTION("methods must be a standard http method") {
        CHECK( r.parse_raw_packet("IMAGINARY /index.html HTTP/1.1\r\n\r\n") == "400" );
        CHECK( r.parse_raw_packet("CONNECTT /index.html HTTP/1.1\r\n\r\n") == "400" );
    }
    SECTION("the http-version must be 1.1") {
        CHECK( r.parse_raw_packet("PUT /somewhere HTTPsomething\r\n\r\n") == "505" );
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.142\r\n\r\n") == "505" );
    }
}

TEST_CASE("invalid headers", "[Http][Request]")
{
    Webserv::Http::Request r;
    CHECK( r.parse_raw_packet("PUT /something HTTP/1.1\r\nHeader: value\r\n  BadHeader: value\r\n\r\n") == "400" );
    CHECK( r.parse_raw_packet("PUT /something HTTP/1.1\r\nContent-length: abcdef\r\n\r\nSome funny message!") == "400" );
    CHECK( r.parse_raw_packet("PUT /something HTTP/1.1\r\nContent-length abcdef\r\n\r\nSome funny message!") == "400" );
    CHECK( r.parse_raw_packet("PUT /something HTTP/1.1\r\nTransfer-Encoding: chunked\r\nSomeFunkyHeader: TastyValue\r\nForgotTheColon Sorry\r\n\r\n19\r\nSome funny message!\r\n0\r\n\r\n") == "400" );
}

TEST_CASE("invalid body", "[Http][Request]")
{
    Webserv::Http::Request r;
    SECTION("body can't exceed allowed max_client_body_size") {
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello", 4) == "413" );
        CHECK( r.parse_raw_packet("PUT /somewhere HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n0\r\n\r\n", 5) == "413" );
    }
    SECTION("body must be correctly indicated by headers") {
        CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\r\n\r\nHello") == "411" );
        CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\r\nContent-Length: 10\r\n\r\nHello") == "400" );
        CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nHello\r\n") == "400" );
    }
    SECTION("body must be chunked if using transfer encoding") {
        CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\r\nTransfer-Encoding: gzip\r\n\r\nsomedata1234") == "501" );
        CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\r\nTransfer-Encoding: fantastic_encoding\r\n\r\n12\r\nsomedata1234") == "501" );
    }
}

TEST_CASE("invalid end of line") {
    Webserv::Http::Request r;
    CHECK( r.parse_raw_packet("GET /index.html HTTP/1.1\r\n") == "400" );
    CHECK( r.parse_raw_packet("GET /index.html HTTP/1.1\r") == "400" );
    CHECK( r.parse_raw_packet("PUT /index.html HTTP/1.1\nDate: yesterday\r\nContent-type: text/plain\r\n\r\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\rContent-type: text/plain\r\n\r\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\n\r\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\r\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\n\r\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\n\n") == "400" );
    CHECK( r.parse_raw_packet("DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\r") == "400" );
}
