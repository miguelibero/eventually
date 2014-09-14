
#include <eventually/http_client.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/thread_dispatcher.hpp>
#include <functional>
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
    	void setopt(CURLoption option, const P& parameter);

    	template<typename... P>
    	void getinfo(CURLINFO info, P... p);
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
    void curl_object::setopt(CURLoption option, const P& parameter)
    {
    	curl_easy_setopt(_curl, option, parameter);
    }

    template<>
    void curl_object::setopt(CURLoption option, const std::string& parameter)
    {
    	curl_easy_setopt(_curl, option, parameter.c_str());
    }

    template<typename... P>
    void curl_object::getinfo(CURLINFO info, P... p)
    {
		curl_easy_getinfo(_curl, info, p...);
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

	http_response http_client::send_dispatched(const http_request& req)
	{
		curl_object curl;
		curl.init();
		curl.setopt(CURLOPT_URL, req.get_url());
		curl.setopt(CURLOPT_FOLLOWLOCATION, 1L);
		curl.setopt(CURLOPT_NOPROGRESS, 1L);
		http_response resp;
        curl.setopt(CURLOPT_WRITEFUNCTION, write_data);
        curl.setopt(CURLOPT_WRITEDATA, &resp);
		curl.perform();
		long http_code = 0;
		curl.getinfo(CURLINFO_RESPONSE_CODE, &http_code);
		resp.set_code(http_code);

		return resp;
	}

	std::future<http_response> http_client::send(const http_request& req)
	{
		return _dispatcher->dispatch(std::bind(&http_client::send_dispatched, this, req));
	}

}

