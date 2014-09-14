

#include <eventually/http_request.hpp>

namespace eventually{

	http_request::http_request(const std::string& url, method m):
	_url(url), _method(m)
	{
	}

	const std::string& http_request::get_url() const
	{
		return _url;
	}

	http_request::method http_request::get_method() const
	{
		return _method;
	}
}