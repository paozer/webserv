#include "ChunkedBody.hpp"

namespace Webserv {
namespace Http {

int ChunkedBody::decode (std::string cb, size_t max_client_body_size)
{
    size_t i = 0;
    int chunk_size = 0;
    if ((i = cb.rfind(CRLF + CRLF)) == std::string::npos || i != cb.length() - 4)
        throw InvalidPacketException("400", "invalid crlf usage");
    while (true) {
        chunk_size = Utils::atoi_base(cb.substr(0, cb.find_first_not_of("0123456789ABCDEF")));
        if ((i = cb.find(CRLF)) == std::string::npos || i != cb.find_first_of("\r\n"))
            throw InvalidPacketException("400", "invalid crlf usage");
        cb.erase(0, i + 2);
        if (chunk_size == 0) {
            _trailer_part = cb;
            break ;
        } else if (chunk_size < 0 || static_cast<size_t>(chunk_size) >= cb.length()) {
            throw InvalidPacketException("400", "invalid chunk size");
        } else if (_body.length() + chunk_size > max_client_body_size) {
            throw InvalidPacketException("413", "chunked body size too large");
        }
        _body += cb.substr(0, chunk_size);
        cb.erase(0, chunk_size + 2);
    }
    return 0;
}

void ChunkedBody::clear (void)
{
    _body.clear();
    _trailer_part.clear();
}

}; // namespace Http
}; // namespace Webserv
