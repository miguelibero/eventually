

#ifndef _eventually_http_request_hpp_
#define _eventually_http_request_hpp_

#include <string>
#include <eventually/http_base.hpp>

namespace eventually{

    class http_request
    {
    public:
        typedef http_data data;
        typedef http_method method;
        typedef http_header header;
        typedef http_headers headers;
    private:
        std::string _url;
        method _method;
        data _body;
        headers _headers;
    public:
        http_request(const std::string& url, method m=method::GET);
        const std::string& get_url() const;
        method get_method() const;
        std::string get_method_str() const;
        static std::string get_method_str(method m);
        const data& get_body() const;
        data& get_body();
        void set_body_str(const std::string& body); 
        size_t get_body_size() const;
        const headers& get_headers() const;
        headers& get_headers();
    };

}

#endif
