

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

	std::string http_request::get_method_str() const
	{
		return get_method_str(_method);
	}

	std::string http_request::get_method_str(method m)
	{
		switch(m)
		{
			case method::GET:
				return "GET";
			case method::POST:
				return "POST";
			case method::PUT:
				return "PUT";
			case method::DELETE:
				return "DELETE";
			default:
				return "";
		}
	}

	const http_request::data& http_request::get_body() const
	{
		return _body;
	}

	http_request::data& http_request::get_body()
	{
		return _body;
	}

	void http_request::set_body_str(const std::string& body)
	{
		_body = data(body.begin(), body.end());
	}

	size_t http_request::get_body_size() const
	{
		return sizeof(data::value_type)*_body.size();
	}
}