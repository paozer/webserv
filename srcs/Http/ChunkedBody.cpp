#include "ChunkedBody.hpp"

namespace Webserv {
namespace Http {

void ChunkedBody::decode (std::string& chunk, size_t max_client_body_size)
{
    size_t i = 0;
    int chunk_size = 0;
    while (true) {
        i = chunk.find(CRLF);
        if (i == std::string::npos || chunk.rfind(CRLF) == i) {
            _state = Incomplete;
            return ;
        }
        if (i != chunk.find_first_of(CRLF)) {
            _state = Error;
            return ;
        }
        chunk_size = Utils::atoi_base(chunk.substr(0, chunk.find_first_of(";" + CRLF)));
        if (chunk_size < 0) {
            _state = Error;
            return ;
        } else if (chunk_size > 0) {
            if (chunk.length() < chunk_size + i + 4) {
                _state = Incomplete;
                return ;
            }
            if (_body.length() + chunk_size > max_client_body_size)
                throw InvalidPacketException("413", "client body size too large");
            chunk.erase(0, i + 2);
            _body += chunk.substr(0, chunk_size);
            chunk.erase(0, chunk_size + 2);
        } else {
            if (chunk.rfind(CRLF) == i) {
                _state = Incomplete;
                return ;
            }
            chunk.erase(0, i + 2);
            _state = Complete;
            _trailer_part = chunk;
            return ;
        }
    }
}

}; // namespace Http
}; // namespace Webserv
