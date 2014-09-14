
#include <eventually/http_client.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/thread_dispatcher.hpp>
#include <functional>
#include <iostream>
#include <curl/curl.h>

namespace eventually {

	http_exception::http_exception(const std::string& desc):
	_description(desc)
	{
	}

    const char* http_exception::what() const throw()
    {
    	return _description.c_str();
    }

    class curl_object
    {
    private:
		CURL* _curl;
		curl_object(const curl_object&);    	
    public:
    	curl_object();
    	~curl_object();
    	void init();
    	CURLcode perform();

    	template<typename P>
    	CURLcode set_opt(CURLoption option, const P& parameter);

    	template<typename... P>
    	void get_info(CURLINFO info, P... p);

    	CURLcode set_headers(const http_headers& headers);
    };

    curl_object::curl_object():
    _curl(nullptr)
    {
    }

    void curl_object::init()
    {
    	_curl = curl_easy_init();    	
		if(!_curl)
		{
			throw new http_exception("could not initialize curl");
		}
    }

    CURLcode curl_object::perform()
    {
    	CURLcode res = curl_easy_perform(_curl);
		if(res != CURLE_OK)
		{
			throw new http_exception(std::string("curl perform failed: ")+ curl_easy_strerror(res));	
		}
		return res;
    }
    
    template<typename P>
    CURLcode curl_object::set_opt(CURLoption option, const P& parameter)
    {
    	return curl_easy_setopt(_curl, option, parameter);
    }

    template<>
    CURLcode curl_object::set_opt(CURLoption option, const std::string& parameter)
    {
    	return curl_easy_setopt(_curl, option, parameter.c_str());
    }

    template<typename... P>
    void curl_object::get_info(CURLINFO info, P... p)
    {
		curl_easy_getinfo(_curl, info, p...);
    }

    CURLcode curl_object::set_headers(const http_headers& headers)
    {
    	curl_slist *list = nullptr;
    	for(const http_header& h : headers)
    	{
    		std::string elm(h.first + ": " + h.second);
    		list = curl_slist_append(list, elm.c_str());
    	}
       	return set_opt(CURLOPT_HTTPHEADER, list); 
    }

    curl_object::~curl_object()
    {
    	if(_curl)
    	{
			curl_easy_cleanup(_curl);
		}
    }

	http_client::http_client(dispatcher* d):
	_dispatcher(d ? d : new thread_dispatcher()),
	_delete_dispatcher(true)
	{
	}

	http_client::http_client(dispatcher& d):
	_dispatcher(&d),
	_delete_dispatcher(false)
	{
	}

	http_client::~http_client()
	{
		if(_delete_dispatcher)
		{
			delete _dispatcher;
		}
	}

	size_t write_data(void* ptr, size_t size, size_t nmemb, http_response* resp)
	{
		size_t n = (size * nmemb);
		http_response::data& rbody = resp->get_body();
		auto rptr = (http_response::data::value_type*)ptr;
		rbody.insert( rbody.end(), rptr, rptr + n );
		return n;
	}

	size_t write_header(char* buffer, size_t size, size_t nitems, http_response* resp)
	{
		size_t n = (size * nitems);
		resp->add_header_str(std::string(buffer, n));
		return n;
	}

	http_response http_client::send_dispatched(const http_request& req)
	{
		curl_object curl;
		curl.init();
		curl.set_opt(CURLOPT_URL, req.get_url());
		curl.set_opt(CURLOPT_FOLLOWLOCATION, true);
		curl.set_opt(CURLOPT_NOPROGRESS, true);
		curl.set_headers(req.get_headers());

		switch(req.get_method())
		{
			case http_request::method::GET:
				curl.set_opt(CURLOPT_HTTPGET, true);
				break;
			case http_request::method::POST:
				curl.set_opt(CURLOPT_POST, true);
				curl.set_opt(CURLOPT_POSTFIELDS, req.get_body().data());
				curl.set_opt(CURLOPT_POSTFIELDSIZE, req.get_body_size());
				break;
			default:
				curl.set_opt(CURLOPT_CUSTOMREQUEST, req.get_method_str());
				break;
		}

		http_response resp;
        curl.set_opt(CURLOPT_WRITEFUNCTION, write_data);
        curl.set_opt(CURLOPT_WRITEDATA, &resp);
        curl.set_opt(CURLOPT_HEADERFUNCTION, write_header);
		curl.set_opt(CURLOPT_HEADERDATA, &resp);

		curl.perform();
		long http_code = 0;
		curl.get_info(CURLINFO_RESPONSE_CODE, &http_code);
		resp.set_code(http_code);

		return resp;
	}

	std::future<http_response> http_client::send(const http_request& req)
	{
		return _dispatcher->dispatch(std::bind(&http_client::send_dispatched, this, req));
	}

}

