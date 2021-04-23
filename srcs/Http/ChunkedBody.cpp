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
        chunk_size = Utils::hex_to_int(chunk.substr(0, chunk.find_first_of(";" + CRLF)));
        if (chunk_size < 0) {
            _state = Error;
            return ;
        } else if (chunk_size > 0) {
            chunk.erase(0, i + 2);
            // add error check is chunk + chunksize != \r\n
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
