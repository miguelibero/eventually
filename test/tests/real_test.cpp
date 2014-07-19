
/*
#include <eventually/thread_dispatcher.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

struct http_request
{
};

struct http_response
{
};

class http_client
{
private:
    thread_dispatcher _dispatcher;

    http_response send_dispatched(connection& conn, http_request& req)
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(0.05f));
        conn.interruption_point();
        std::this_thread::sleep_for(std::chrono::duration<float>(0.05f));
        return http_response();
    }

public:
    std::future<http_response> send(connection& conn, const http_request& req)
    {
        return _dispatcher.dispatch(conn, std::bind(&http_client::send_dispatched, this, conn, req));
    }
};

class widget
{
private:
    scoped_connection _http_conn;
    http_client& _http_client;
    dispatcher& _main_dispatcher;
    bool _finished;

    void on_http_response(http_response resp)
    {
        _finished = true;
    }

public:
    widget(http_client& http_client, dispatcher& main_dispatcher):
    _http_client(http_client), _main_dispatcher(main_dispatcher), _finished(false)
    {
    }

    void init()
    {
        http_request req;
        _main_dispatcher.then(_http_conn, _http_client.send(_http_conn, req),
            [this](http_response resp){ on_http_response(resp); });
    }
};

TEST(real, http_client) {

    http_client c;
    dispatcher m;

    {
        widget w(c, m);
        w.init();
    }

}

*/