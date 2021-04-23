#include "catch.hpp"
#include "../../srcs/Http/Request.hpp"

namespace Webserv {
namespace Http {

//TODO check if trailer part contains illegal headers
TEST_CASE("request reconstruction", "[Http][Request]") {
    Request r;
    SECTION("") {
        r.append("PUT /something HTT");
        REQUIRE( r.get_state() == Requestline );
        r.append("P/1.1");
        REQUIRE( r.get_state() == Requestline );
        r.append("\r\n");
        REQUIRE( r.get_state() == Headers );
        CHECK( r.get_method() == "PUT" );
        CHECK( r.get_uri() == "/something" );
        r.append("someheader: somevalue\r");
        REQUIRE( r.get_state() == Headers );
        r.append("\nsomething_else:somevalue1\r\nheader: val");
        REQUIRE( r.get_state() == Headers );
        REQUIRE( r.has_header("someheader") );
        CHECK( r.get_header_values("someheader") == "somevalue" );
        REQUIRE( r.has_header("something_else") );
        CHECK( r.get_header_values("something_else") == "somevalue1" );
        r.append("ue\r\n");
        REQUIRE( r.get_state() == Headers );
        REQUIRE( r.has_header("header") );
        CHECK( r.get_header_values("header") == "value" );
        r.append("\r\n");
        REQUIRE( r.get_state() == Complete );
    }
    SECTION("") {
        r.append("POST / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Go-http-client/1.1\r\nTransfer-Encoding: chunked\r\nContent-");
        REQUIRE( r.get_state() == Headers );
        r.append("Type: test/file\r\nAccept-Encoding: gzip\r\n\r\n");
        REQUIRE( r.get_state() == Body );
        r.append("0\r\n\r\n");
        REQUIRE( r.get_state() == Complete );
    }
}

/* TESTING PARSING OF VALID REQUESTS */
TEST_CASE("valid request lines", "[Http][Request]")
{
    SECTION("all standard request methods are recognized") {
        std::vector<std::string> methods = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS" };
        for (auto it = methods.begin(); it != methods.end(); ++it) {
            Request r;
            r.append(*it + " /index.html HTTP/1.1\r\n\r\n");
            REQUIRE( r.get_state() == Complete );
            CHECK( r.get_method() == *it );
        }
    }
    SECTION("URI are parsed correctly") {
        Request r;
        std::vector<std::pair<std::string, std::string> > arr = {
            { "PUT /something HTTP/1.1\r\n\r\n", "/something" },
            { "GET / HTTP/1.1\r\n\r\n", "/" },
            { "DELETE http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1\r\n\r\n", "http://www.w3.org/pub/WWW/TheProject.html" }
        };
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            Request r;
            r.append(it->first);
            REQUIRE( r.get_state() == Complete );
            CHECK( r.get_uri() == it->second );
        }
    }
}

TEST_CASE("valid headers", "[Http][Request]")
{
    Request r;
    std::vector<HeaderValues> values = { {"$va1lue"}, {"2value!"}, {"~value3"}, {"_something.*, 012345abc#"} };
    r.append("PUT /something HTTP/1.1\r\nHeader: $va1lue   \r\n2HeadeR:   2value!\r\nThIrdHeaDer:~value3\r\nFOURTHHEADER:    _something.*, 012345abc# \r\n\r\n");
    REQUIRE( r.get_state() == Complete );
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
    SECTION("simple message body is parsed correctly") {
        std::vector<std::pair<std::string, std::string> > arr = {
            { "PUT /somewhere HTTP/1.1\r\nContent-Length:  12\r\n\r\nHello World!", "Hello World!" },
            { "PUT /somewhere HTTP/1.1\r\nContent-Length:12 \r\n\r\nHello\nWorld!", "Hello\nWorld!" },
            { "PUT /somewhere HTTP/1.1\r\nContent-Length:  12  \r\n\r\nHello\rWorld!", "Hello\rWorld!" },
        };
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            Request r;
            r.append(it->first);
            REQUIRE( r.get_state() == Complete );
            CHECK( r.get_body() == it->second );
        }
    }
    SECTION("content-length header is removed when is body chunked") {
        Request r;
        r.append("PUT /somewhere HTTP/1.1\r\nContent-Length:  10 \r\nTransfer-Encoding: chunked   \r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n000;ext=value\r\n\r\n");
        REQUIRE( r.get_state() == Complete );
        CHECK_FALSE( r.has_header("Content-Length") );
        CHECK( r.has_header("Transfer-Encoding") );
        CHECK( r.get_body() == "Hello!" );
    }
    SECTION("headers in trailer part are parsed correctly") {
        Request r0;
        r0.append("PUT /somewhere HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n000;ext=value\r\ntrailer-header: somevalue \r\n\r\n");
        REQUIRE( r0.has_header("Trailer-Header") );
        CHECK( r0.get_header_values("Trailer-Header") == "somevalue" );

        Request r1;
        r1.append("PUT /somefile HTTP/1.1\r\nTransfer-Encoding:   chunked  \r\n\r\n6\r\n#!some\r\n12\r\n executable \r\n6\r\nscript\r\n000;ext1=value1;ext2=value2;ext3\r\ntrailer-header: somevalue \r\nother-trailer-header:someothervalue\r\nsomemoreheader: values          \r\n\r\n");
        CHECK( r1.get_body() == "#!some executable script" );
        REQUIRE( r1.has_header("Trailer-Header") );
        CHECK( r1.get_header_values("Trailer-Header") == "somevalue" );
        REQUIRE( r1.has_header("Other-Trailer-Header") );
        CHECK( r1.get_header_values("Other-Trailer-Header") == "someothervalue" );
        REQUIRE( r1.has_header("SOMEMOREHEADER") );
        CHECK( r1.get_header_values("SOMEMOREHEADER") == "values" );
    }
}

/* TESTING DETECTION OF INVALID REQUESTS */
TEST_CASE("invalid request lines", "[Http][Request]")
{
    std::vector<std::string> arr;
    SECTION("single spaces separate request line parameters") {
        arr = {
            "GET  /index.html HTTP/1.1\r\n\r\n",
            "HEAD             /somewhere          HTTP/1.1\r\n\r\n",
            "PUT /somewhere HTTP/1.1 \r\n\r\n",
            "PUT /index.html  HTTP/1.1\r\n\r\n",
            "GET     /index.html HTTP/1.1\r\n\r\n",
        };
    }
    SECTION("three parameters compose a request line") {
        arr = {
            "\r\n\r\n",
            "GET \r\n\r\n",
            "GET HTTP/1.1\r\n\r\n",
            "GET HTTP/1.1 \r\n\r\n",
            "PUT /somewhere HTTP/1.1 maliciousstuff somemore\r\n\r\n",
            "PUT /somewhere HTTP/1.1 maliciousstuff\r\n\r\n",
        };
    }
    SECTION("methods must be a standard http method") {
        arr = {
            "IMAGINARY /index.html HTTP/1.1\r\n\r\n",
            "CONNECTT /index.html HTTP/1.1\r\n\r\n",
        };
    }
    SECTION("the http-version must be 1.1") {
        arr = {
            "PUT /somewhere HTTPsomething\r\n\r\n",
            "PUT /somewhere HTTP/1.142\r\n\r\n",
        };
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(*it);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid headers", "[Http][Request]")
{
    std::vector<std::string> arr = {
        "PUT /something HTTP/1.1\r\nHeader: value\r\n  BadHeader: value\r\n\r\n",
        "PUT /something HTTP/1.1\r\nContent-length: abcdef\r\n\r\nSome funny message!",
        "PUT /something HTTP/1.1\r\nContent-length abcdef\r\n\r\nSome funny message!",
        "PUT /something HTTP/1.1\r\nTransfer-Encoding: chunked\r\nSomeFunkyHeader: TastyValue\r\nForgotTheColon Sorry\r\n\r\n19\r\nSome funny message!\r\n0\r\n\r\n"
    };
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(*it);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid body", "[Http][Request]")
{
    std::vector<std::pair<std::string, std::string> > arr;
    SECTION("body can't exceed allowed max_client_body_size") {
        arr = {
            //{ "PUT /somewhere HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello", "413" },
            //{ "PUT /somewhere HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n0\r\n\r\n", "413" },
        };
    }
    SECTION("body must be correctly indicated by headers") {
        arr = {
            { "PUT /index.html HTTP/1.1\r\n\r\nHello", "411" },
            //{ "PUT /index.html HTTP/1.1\r\nContent-Length: 10\r\n\r\nHello", "400" },
            { "PUT /index.html HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nHello\r\n", "400" },
        };
    }
    SECTION("body must be chunked if using transfer encoding") {
        arr = {
            { "PUT /index.html HTTP/1.1\r\nTransfer-Encoding: gzip\r\n\r\nsomedata1234", "501" },
            { "PUT /index.html HTTP/1.1\r\nTransfer-Encoding: fantastic_encoding\r\n\r\n12\r\nsomedata1234", "501" },
        };
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(it->first);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid end of line") {
    std::vector<std::pair<std::string, State> > arr = {
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\n\n", Error },
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\r", Error },
        { "PUT /index.html HTTP/1.1\nDate: yesterday\r\nContent-type: text/plain\r\n\r\n", Error },
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\rContent-type: text/plain\r\n\r\n", Error },
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\n\r\n", Error },
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\r\r\n", Error },
        { "DELETE somefile HTTP/1.1\r\nDate: yesterday\nContent-type: text/plain\n\r\n", Error }
    };
    for (auto it  = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(it->first);
        CHECK( r.get_state() == it->second );
    }
}

};
};
