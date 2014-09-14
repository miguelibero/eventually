#ifndef _eventually_http_client_hpp_
#define _eventually_http_client_hpp_

#include <future>
#include <exception>
#include <string>

namespace eventually{

    class dispatcher;
    class connection;
    class http_request;
    class http_response;

    /**
     * The exception thrown 
     */
    class http_exception : public std::exception
    {
    private:
        std::string _description;
    public:
        http_exception(const std::string& desc);
        virtual const char* what() const throw();
    };

    class http_client
    {
    private:
        dispatcher* _dispatcher;
        bool _delete_dispatcher;

        http_response send_dispatched(connection& c, const http_request& req);

    public:

        http_client(dispatcher* d=nullptr);
        http_client(dispatcher& d);

        ~http_client();

        dispatcher& get_dispatcher();
        std::future<http_response> send(const http_request& req);
        std::future<http_response> send(connection& c, const http_request& req);
    };

}

#endif