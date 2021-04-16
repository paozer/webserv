#include "../srcs/Http/ChunkedBody.hpp"
#include "catch.hpp"

TEST_CASE("empty chunked-body", "[ChunkedBody][Http]")
{
    Webserv::Http::ChunkedBody cb;
    REQUIRE_NOTHROW( cb.decode("0\r\n\r\n") );
    CHECK( cb.get_body() == "" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("000000\r\n\r\n") );
    CHECK( cb.get_body() == "" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("0;chunk-ext\r\n\r\n") );
    CHECK( cb.get_body() == "" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("0;chunk-ext=value\r\n\r\n") );
    CHECK( cb.get_body() == "" );
}

TEST_CASE("non-empty chunked-body", "[ChunkedBody][Http]")
{
    Webserv::Http::ChunkedBody cb;
    REQUIRE_NOTHROW( cb.decode("19\r\nThis does not look \r\n19\r\nlike a duplicate of\r\n7\r\n those.\r\n0\r\n\r\n") );
    CHECK( cb.get_body() == "This does not look like a duplicate of those." );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("1\r\nw\r\n3\r\nooo\r\n1\r\nw\r\n000\r\n\r\n") );
    CHECK( cb.get_body() == "wooow" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("5\r\nHello\r\n6\r\n World\r\n1\r\n!\r\n00000\r\n\r\n") );
    CHECK( cb.get_body() == "Hello World!" );
}

TEST_CASE("trailer-part", "[ChunkedBody][Http]")
{
    Webserv::Http::ChunkedBody cb;
    REQUIRE_NOTHROW( cb.decode("0\r\nDate:Sun, 03 Jan 3030 08:49:37 GMT\r\n\r\n") );
    CHECK( cb.get_trailer_part() == "Date:Sun, 03 Jan 3030 08:49:37 GMT\r\n\r\n" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("5\r\nHello\r\n6\r\n World\r\n1\r\n!\r\n00000\r\nDate:   Sun, 03 Jan 3030 08:49:37 GMT\r\n\r\n") );
    CHECK( cb.get_body() == "Hello World!" );
    CHECK( cb.get_trailer_part() == "Date:   Sun, 03 Jan 3030 08:49:37 GMT\r\n\r\n" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("4;ext=val\r\nTest\r\n00000\r\nDate: today\r\nExpires: now\r\n\r\n") );
    CHECK( cb.get_body() == "Test" );
    CHECK( cb.get_trailer_part() == "Date: today\r\nExpires: now\r\n\r\n" );
    cb.clear();
    REQUIRE_NOTHROW( cb.decode("2\r\nHe\r\n2\r\nll\r\n2\r\no!\r\n000;ext=value\r\ntrailer-header: somevalue \r\n\r\n") );
    CHECK( cb.get_body() == "Hello!" );
    CHECK( cb.get_trailer_part() == "trailer-header: somevalue \r\n\r\n" );
}

TEST_CASE("missing last-chunk", "[ChunkedBody][Http]")
{
    Webserv::Http::ChunkedBody cb;
    CHECK_THROWS_AS( cb.decode("4;ext=val\r\nTest\r\nDate: today\r\nExpires: now\r\n\r\n"), Webserv::Http::InvalidPacketException );
    cb.clear();
    CHECK_THROWS_AS( cb.decode("4;ext=val\r\nTest\r\n"), Webserv::Http::InvalidPacketException );
    cb.clear();
}

TEST_CASE("invalid crlf usage")
{
    Webserv::Http::ChunkedBody cb;
    CHECK_THROWS_AS( cb.decode("0\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("0\r"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("0\r\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("0\n\r\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("3\r\nHi \r\n7\r\nhow are\r5\r\n you!\r\n\r\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("3\r\nHi \r\n7\r\nhow are\r\n5\r you!\r\n\r\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("3\r\nHi \r\n7\r\nhow are\r5\r\n you!\r\r\n"), Webserv::Http::InvalidPacketException );
    CHECK_THROWS_AS( cb.decode("3\r\nHi \r\n7\r\nhow are\r\n5\r\n you!\r\n"), Webserv::Http::InvalidPacketException );
}
