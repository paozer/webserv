#include "catch.hpp"
#include "../../srcs/Http/Response.hpp"

TEST_CASE("building HttpResponse", "[HttpResponse]")
{
    Webserv::Http::Response r;

    r.set_status_code("400");
    r.build_raw_packet();

    CHECK( r.get_raw_packet() == "HTTP/1.1 400 Bad Request\r\n\r\n" );
    r.append_header("Server", "webserv");
    r.append_header("Accept-Encoding", "chunked");
    r.append_header("Connection", "keep-alive");
    r.append_header("Accept-Encoding", "gzip");
    r.build_raw_packet();
    CHECK( r.get_raw_packet() == "HTTP/1.1 400 Bad Request\r\nAccept-Encoding: chunked, gzip\r\nConnection: keep-alive\r\nServer: webserv\r\n\r\n" );
    r.set_body("5\r\nHello\r\n");
    r.build_raw_packet();
    CHECK( r.get_raw_packet() == "HTTP/1.1 400 Bad Request\r\nAccept-Encoding: chunked, gzip\r\nConnection: keep-alive\r\nServer: webserv\r\n\r\n5\r\nHello\r\n" );
}
