#include "catch.hpp"
#include "../../srcs/Http/Response.hpp"

TEST_CASE("building HttpResponse", "[HttpResponse]")
{
    Webserv::Http::Response r;
    std::string s ("Hi! This is a body.\nIt contains newlines and a null-character\0 also spaces   ", 78);

    r.set_status_code("200");
    r.append_header("Accept-Encoding", "chunked");
    r.append_header("Server", "webserv");
    r.append_header("Connection", "close");
    r.append_header("Accept-Encoding", "gzip");
    r.set_body(s);
    r.set_content_length();
    r.build_raw_packet();

    CHECK( r.get_raw_packet() == "HTTP/1.1 200 OK\r\nAccept-Encoding: chunked, gzip\r\nConnection: close\r\nContent-Length: " +
                                 std::to_string(s.length()) + "\r\nServer: webserv\r\n\r\n" + s
    );
}
