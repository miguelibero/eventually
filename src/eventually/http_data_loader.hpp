#ifndef _eventually_http_data_loader_hpp_
#define _eventually_http_data_loader_hpp_

#include <eventually/data_loader.hpp>
#include <functional>

namespace eventually {

	class http_client;
    class http_request;
    class http_response;
    class connection;
    class dispatcher;

	class http_data_loader
	{
    public:
        typedef std::function<http_request(const std::string& name)> request_create;
    private:
        http_client* _client;
        bool _delete_client;
        request_create _request_create;

        data when_sent(connection& c, http_response&& resp);
        http_request create_request(const std::string& name);

    public:

        http_data_loader(http_client* c=nullptr);
        http_data_loader(http_client& c);

        ~http_data_loader();

        void set_request_create(const request_create& create);
        dispatcher& get_dispatcher();
        http_client& get_client();
        std::future<data> load(connection& c, const std::string& name);
        std::future<data> load(const std::string& name);
	};
}

#endif