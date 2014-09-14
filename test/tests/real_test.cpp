
#include <eventually/thread_dispatcher.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <eventually/http_client.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

class widget
{
private:
    scoped_connection _http_conn;
    http_client& _http_client;
    dispatcher& _main_dispatcher;
    bool _finished;

    void on_http_response(const http_response& resp)
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
        http_request req("http://httpbin.org");
        _main_dispatcher.when(_http_conn,
            std::bind(&widget::on_http_response, this, std::placeholders::_1),
            _http_client.send(_http_conn, req));
    }

    bool finished() const
    {
        return _finished;
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