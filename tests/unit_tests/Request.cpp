#include "catch.hpp"
#include "../../srcs/Http/Request.hpp"

namespace Webserv {
namespace Http {

TEST_CASE("reconstructing streamed requests", "[Http][Request]") {
    Request r;
    SECTION("no body") {
        r.append("PUT /something HTT");
        REQUIRE( r.get_state() == Requestline );
        r.append("P/1.1");
        REQUIRE( r.get_state() == Requestline );
        r.append("\r\nHost: somehost");
        REQUIRE( r.get_state() == Headers );
        r.append("\r\nsomeheader: somevalue\r");
        REQUIRE( r.get_state() == Headers );
        r.append("\nsomething_else:somevalue1\r\nheader: val");
        REQUIRE( r.get_state() == Headers );
        r.append("ue\r\n");
        REQUIRE( r.get_state() == Headers );
        r.append("\r\n");
        REQUIRE( r.get_state() == Complete );

        CHECK( r.get_method() == "PUT" );
        CHECK( r.get_uri() == "/something" );
        REQUIRE( r.has_header("header") );
        CHECK( r.get_header_values("header") == "value" );
        REQUIRE( r.has_header("someheader") );
        CHECK( r.get_header_values("someheader") == "somevalue" );
        REQUIRE( r.has_header("something_else") );
        CHECK( r.get_header_values("something_else") == "somevalue1" );
    }
    SECTION("transfer encoded") {
        r.append("POST / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Go-http-c");
        REQUIRE( r.get_state() == Headers );
        r.append("lient/1.1\r\nTransfer-Encoding:   chunked\r\nContent-");
        REQUIRE( r.get_state() == Headers );
        r.append("Type: test/file\r\nAccept-Encoding:   gzip, chunked   \r\n");
        REQUIRE( r.get_state() == Headers );
        r.append("\r\n");
        REQUIRE( r.get_state() == Body );
        r.append("5\r\nHello\r\n");
        REQUIRE( r.get_state() == Body );
        r.append("0");
        REQUIRE( r.get_state() == Body );
        r.append("\r\n");
        REQUIRE( r.get_state() == Body );
        r.append("\r\n");
        REQUIRE( r.get_state() == Complete );

        CHECK( r.get_method() == "POST" );
        CHECK( r.get_uri() == "/" );
        REQUIRE( r.has_header("HOST") );
        CHECK( r.get_header_values("HoSt") == "localhost:8080" );
        REQUIRE( r.has_header("USer-agENT") );
        CHECK( r.get_header_values("USer-agENT") == "Go-http-client/1.1" );
        REQUIRE( r.has_header("Transfer-ENCODING") );
        CHECK( r.get_header_values("TRANSFER-Encoding") == "chunked" );
        REQUIRE( r.has_header("CONTENT-TYPE") );
        CHECK( r.get_header_values("CONTENT-TYPE") == "test/file" );
        REQUIRE( r.has_header("Accept-ENCODING") );
        CHECK( r.get_header_values("ACCEPT-Encoding") == "gzip, chunked" );
        CHECK( r.get_body() == "Hello" );
    }
    SECTION("content-length") {
        r.append("PUT /somefunkyfile HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Go-http-c");
        REQUIRE( r.get_state() == Headers );
        r.append("lient/1.1\r\nContent-Length:   10\r\nContent-");
        REQUIRE( r.get_state() == Headers );
        r.append("Type: test/file\r\nAccept-Encoding:   gzip, chunked   \r\n");
        REQUIRE( r.get_state() == Headers );
        r.append("\r\n");
        REQUIRE( r.get_state() == Body );
        r.append("0123");
        REQUIRE( r.get_state() == Body );
        r.append("4");
        REQUIRE( r.get_state() == Body );
        r.append("5");
        REQUIRE( r.get_state() == Body );
        r.append("");
        REQUIRE( r.get_state() == Body );
        r.append("6");
        REQUIRE( r.get_state() == Body );
        r.append("789");
        REQUIRE( r.get_state() == Complete );

        CHECK( r.get_method() == "PUT" );
        CHECK( r.get_uri() == "/somefunkyfile" );
        REQUIRE( r.has_header("HOST") );
        CHECK( r.get_header_values("HoSt") == "localhost:8080" );
        REQUIRE( r.has_header("USer-agENT") );
        CHECK( r.get_header_values("USer-agENT") == "Go-http-client/1.1" );
        REQUIRE( r.has_header("CONTENT-length") );
        CHECK( r.get_header_values("content-LENGTH") == "10" );
        REQUIRE( r.has_header("CONTENT-TYPE") );
        CHECK( r.get_header_values("CONTENT-TYPE") == "test/file" );
        REQUIRE( r.has_header("Accept-ENCoDING") );
        CHECK( r.get_header_values("ACCEPT-EncOding") == "gzip, chunked" );
        CHECK( r.get_body() == "0123456789" );
    }
}

TEST_CASE("invalid request lines", "[Http][Request]")
{
    std::vector<std::string> arr;
    SECTION("single spaces separate request line parameters") {
        arr = {
            "GET  /index.html HTTP/1.1\r\n",
            "HEAD             /somewhere          HTTP/1.1\r\n",
            "PUT /somewhere HTTP/1.1 \r\n",
            "PUT /index.html  HTTP/1.1\r\n",
            "GET     /index.html HTTP/1.1\r\n",
        };
    }
    SECTION("three parameters compose a request line") {
        arr = {
            "\r\n",
            "GET \r\n",
            "GET HTTP/1.1\r\n",
            "GET HTTP/1.1 \r\n",
            "PUT /somewhere HTTP/1.1 maliciousstuff somemore\r\n",
            "PUT /somewhere HTTP/1.1 maliciousstuff\r\n",
        };
    }
    SECTION("methods must be a standard http method") {
        arr = {
            "IMAGINARY /index.html HTTP/1.1\r\n",
            "CONNECTT /index.html HTTP/1.1\r\n",
        };
    }
    SECTION("the http-version must be 1.1") {
        arr = {
            "PUT /somewhere HTTPsomething\r\n",
            "PUT /somewhere HTTP/1.142\r\n",
        };
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(*it);
        r.append("Host: host\r\n\r\n");
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid headers", "[Http][Request]")
{
    std::vector<std::string> arr = {
        "  BadHeader: value\r\n\r\n",
        "Content-length: abcdef\r\n\r\nSome funny message!",
        "Content-length abcdef\r\n\r\nSome funny message!",
        "Transfer-Encoding: chunked\r\nSomeFunkyHeader: TastyValue\r\nForgotTheColon Sorry\r\n\r\n19\r\nSome funny message!\r\n0\r\n\r\n"
    };
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append("PUT /something HTTP/1.1\r\nHost: value\r\n");
        r.append(*it);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid body", "[Http][Request]")
{
    std::vector<std::pair<std::string, State> > arr;
    SECTION("body must be correctly indicated by headers") {
        arr = {
            { "\r\nHello", Error },
            { "Transfer-Encoding: chunked\r\n\r\nHello\r\n", Error},
            { "Content-Length: 9\r\n\r\nThis body is way too long!", Error},
        };
    }
    SECTION("body must be chunked if using transfer encoding") {
        arr = {
            { "Transfer-Encoding: gzip\r\n\r\nsomedata1234", Error },
            { "Transfer-Encoding: fantastic_encoding\r\n\r\n12\r\nsomedata1234", Error },
        };
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append("PUT /index.html HTTP/1.1\r\nhost: hostihost\r\n");
        r.append(it->first);
            CHECK( r.get_state() == it->second );
    }
}

TEST_CASE("invalid end of line") {
    std::vector<std::string> arr = {
        { "DELETE somefile HTTP/1.1\nDate: yesterday\r\nContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\rContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\n\nContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\r\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\n\r\n" }
    };
    for (auto it  = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(*it);
        CHECK( r.get_state() == Error );
    }
}

};
};
