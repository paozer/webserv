#include "ChunkedBody.hpp"

namespace Webserv {
namespace Http {

ChunkedBody::ChunkedBody  (void)
    : curr_chunk_size(-1), state(Incomplete)
{
}

void ChunkedBody::decode (std::string& chunk)
{
    size_t i;
    while (!chunk.empty()) {
        if (curr_chunk_size == -1) {
            i = chunk.find(CRLF);
            if (i == std::string::npos)
                return ;
            curr_chunk_size = Utils::atoi_base(chunk.substr(0, chunk.find_first_of(CRLF + ";")));
            curr_chunk_size += 2 * (curr_chunk_size > 0);
            chunk.erase(0, i + 2);
            if (curr_chunk_size < 0) {
                state = Error;
                return ;
            }
        } else if (curr_chunk_size > 0) {
            if (curr_chunk_size > 2)
                body += chunk.substr(0, curr_chunk_size - 2);
            i = curr_chunk_size;
            curr_chunk_size -= std::min(chunk.length(), static_cast<size_t>(curr_chunk_size));
            curr_chunk_size -= 1 * (curr_chunk_size == 0);
            chunk.erase(0, i);
        } else if (curr_chunk_size == 0) {
            if ((i = chunk.find(CRLF)) == 0)
                state = Complete;
            return ;
        }
    }
}

}; // namespace Http
}; // namespace Webserv
