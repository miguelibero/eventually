

#ifndef _eventually_http_response_hpp_
#define _eventually_http_response_hpp_

#include <string>
#include <eventually/http_base.hpp>

namespace eventually{

	class http_response
	{
	public:
		typedef http_data data;
		typedef http_header header;
		typedef http_headers headers;
	private:
		data _body;
		int _code;
		headers _headers;
	public:
		http_response(const data& body=data(), int code=200);
		const data& get_body() const;
		data& get_body();
		std::string get_body_str() const;
		int get_code() const;
		void set_code(int code);
		const headers& get_headers() const;
		headers& get_headers();
		bool add_header_str(const std::string& str);
		headers::const_iterator get_header(const std::string& name) const;
	};

}

#endif