#include "ChunkedBody.hpp"

namespace Webserv {
namespace Http {

State ChunkedBody::decode (std::string chunk, size_t max_client_body_size)
{
    _body.clear();
    _trailer_part.clear();
    size_t i = 0;
    int chunk_size = 0;
    while (true) {
        chunk_size = Utils::atoi_base(chunk.substr(0, chunk.find_first_of(";\r\n")));
        if ((i = chunk.find(CRLF)) == std::string::npos || i != chunk.find_first_of("\r\n")) {
            if (chunk.empty())
                break ;
            throw InvalidPacketException("400", "invalid crlf usage");
        }
        chunk.erase(0, i + 2);
        if (chunk_size == 0) {
            _trailer_part = chunk;
            return Complete;
        } else if (chunk_size < 0 || static_cast<size_t>(chunk_size) >= chunk.length()) {
            throw InvalidPacketException("400", "invalid chunk size");
        } else if (_body.length() + chunk_size > max_client_body_size) {
            throw InvalidPacketException("413", "chunked body size too large");
        }
        _body += chunk.substr(0, chunk_size);
        chunk.erase(0, chunk_size + 2);
    }
    return Incomplete;
}

}; // namespace Http
}; // namespace Webserv
