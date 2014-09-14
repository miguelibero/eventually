

#ifndef _eventually_http_response_hpp_
#define _eventually_http_response_hpp_

#include <vector>
#include <string>
#include <cstdint>

namespace eventually{

	class http_response
	{
	public:
		typedef std::vector<uint8_t> data;
	private:
		data _body;
		int _code;
	public:
		http_response(const data& body=data(), int code=200);
		const data& get_body() const;
		data& get_body();
		std::string get_body_str() const;
		int get_code() const;
		void set_code(int code);
	};

}

#endif