

#ifndef _eventually_http_request_hpp_
#define _eventually_http_request_hpp_

#include <string>
#include <vector>
#include <cstdint>

namespace eventually{

	enum class http_request_method
	{
		GET,
		POST,
		PUT,
		DELETE
	};

	class http_request
	{
	public:
		typedef std::vector<uint8_t> data;
		typedef http_request_method method;
	private:
		std::string _url;
		method _method;
		data _body;
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
	};

}

#endif