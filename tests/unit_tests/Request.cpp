#include "catch.hpp"
#include "../../srcs/Http/Request.hpp"

namespace Webserv {
namespace Http {

TEST_CASE("reconstructing requests read from different parts", "[Http][Request]") {
    Request r;
    SECTION("no body") {
        r.append("PUT /something HTT", 100);
        REQUIRE( r.get_state() == Requestline );
        r.append("P/1.1", 100);
        REQUIRE( r.get_state() == Requestline );
        r.append("\r\nHost: somehost", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("\r\nsomeheader: somevalue\r", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("\nsomething_else:somevalue1\r\nheader: val", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("ue\r\n", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("\r\n", 100);
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
    SECTION("transfer encoded body") {
        r.append("POST / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Go-http-c", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("lient/1.1\r\nTransfer-Encoding:   chunked\r\nContent-", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("Type: test/file\r\nAccept-Encoding:   gzip, chunked   \r\n", 100);
        REQUIRE( r.get_state() == Headers );
        r.append("\r\n", 100);
        REQUIRE( r.get_state() == Body );
        r.append("5\r\nHello\r\n", 100);
        REQUIRE( r.get_state() == Body );
        r.append("0", 100);
        REQUIRE( r.get_state() == Body );
        r.append("\r\n", 100);
        REQUIRE( r.get_state() == Body );
        r.append("\r\n", 100);
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
        r.append(*it, 100);
        r.append("Host: host\r\n\r\n", 100);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid headers", "[Http][Request]")
{
    std::vector<std::string> arr = {
        "  BadHeader: value\r\n\r\n",
        "Content-length: abcdef\r\n\r\nSome funny message!", // TODO
        "Content-length abcdef\r\n\r\nSome funny message!",
        "Transfer-Encoding: chunked\r\nSomeFunkyHeader: TastyValue\r\nForgotTheColon Sorry\r\n\r\n19\r\nSome funny message!\r\n0\r\n\r\n"
    };
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append("PUT /something HTTP/1.1\r\nHost: value\r\n", 100);
        r.append(*it, 100);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid body", "[Http][Request]")
{
    std::vector<std::string> arr;
    SECTION("body can't exceed allowed max_client_body_size") {
        arr = {
            { "Content-Length: 21\r\n\r\nHello this is to long" },
            { "Transfer-Encoding: chunked\r\n\r\n6\r\nHello \r\n10\r\nthis is to\r\n5\r\n long\r\n0\r\n\r\n" },
        };
    }
    SECTION("body must be correctly indicated by headers") {
        arr = {
            { "\r\nHello" },
            { "Content-Length: 9\r\n\r\nHello" }, // TODO
            { "Transfer-Encoding: chunked\r\n\r\nHello\r\n" }, // TODO
        };
    }
    SECTION("body must be chunked if using transfer encoding") {
        arr = {
            { "Transfer-Encoding: gzip\r\n\r\nsomedata1234" },
            { "Transfer-Encoding: fantastic_encoding\r\n\r\n12\r\nsomedata1234" },
        };
    }
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append("PUT /index.html HTTP/1.1\r\nhost: hostihost\r\n", 10);
        r.append(*it, 10);
        CHECK( r.get_state() == Error );
    }
}

TEST_CASE("invalid end of line") {
    std::vector<std::string> arr = {
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\n\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\r\r" },
        { "DELETE somefile HTTP/1.1\nDate: yesterday\r\nContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\rContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\r\n\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\r\r\n" },
        { "DELETE somefile HTTP/1.1\r\nHost: localhost:8080\r\nDate: yesterday\r\nContent-type: text/plain\n\r\n" }
    };
    for (auto it  = arr.begin(); it != arr.end(); ++it) {
        Request r;
        r.append(*it, 100);
        CHECK( r.get_state() == Error );
    }
}

};
};
