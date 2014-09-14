

#include <eventually/http_response.hpp>
#include <utility>

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
}

