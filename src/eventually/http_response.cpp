

#include <eventually/http_response.hpp>
#include <utility>
#include <algorithm>

namespace eventually{

    http_response::http_response(const data& body, int code):
    _body(body), _code(code)
    {
    }

    const http_response::data& http_response::get_body() const
    {
        return _body;
    }

    http_response::data& http_response::get_body()
    {
        return _body;
    }

    std::string http_response::get_body_str() const
    {
        return std::string(_body.begin(), _body.end());
    }

    int http_response::get_code() const
    {
        return _code;
    }

    void http_response::set_code(int code)
    {
        _code = code;
    }

    const http_response::headers& http_response::get_headers() const
    {
        return _headers;
    }

    http_response::headers& http_response::get_headers()
    {
        return _headers;
    }

    inline void trim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    bool http_response::add_header_str(const std::string& str)
    {
        size_t p = str.find(":");
        if(p == std::string::npos)
        {
            return false;
        }
        header h{ str.substr(0, p), str.substr(p+1) };
        trim(h.first);
        trim(h.second);
        _headers.push_back(h);
        return true;
    }

    http_response::headers::const_iterator http_response::find_header(const std::string& name) const
    {
        return std::find_if(_headers.begin(), _headers.end(), [&name](const header& h){
            return h.first == name;
        });
    }
}

