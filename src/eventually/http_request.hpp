

#ifndef _eventually_http_request_hpp_
#define _eventually_http_request_hpp_

#include <string>

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
		typedef http_request_method method;
	private:
		std::string _url;
		method _method;
	public:
		http_request(const std::string& url, method m=method::GET);
		const std::string& get_url() const;
		method get_method() const;
	};

}

#endif