

#ifndef _eventually_http_base_hpp_
#define _eventually_http_base_hpp_

#include <vector>
#include <utility>
#include <cstdint>

namespace eventually
{
    enum class http_method
    {
        GET,
        POST,
        PUT,
        DELETE
    };

    typedef std::vector<uint8_t> http_data;
    typedef std::pair<std::string, std::string> http_header;
    typedef std::vector<http_header> http_headers;
}

#endif
