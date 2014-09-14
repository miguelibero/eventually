
#include <eventually/http_client.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/connection.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

TEST(http_client, basic) {

    http_client client;
    http_request req("http://httpbin.org/get");

    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_body().size());
    ASSERT_NE(-1, resp.get_body_str().find("headers"));
    ASSERT_EQ(200, resp.get_code());
}

TEST(http_client, post) {

	http_client client;
    http_request req("http://httpbin.org/post", http_method::POST);
    req.set_body_str("test_eventually");

    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_body().size());
    ASSERT_NE(-1, resp.get_body_str().find("test_eventually"));
    ASSERT_EQ(200, resp.get_code());
}

TEST(http_client, request_headers) {

    http_client client;
    http_request req("http://httpbin.org/headers");
    req.get_headers().push_back({"X-Eventually", "true"});
    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_body().size());
    ASSERT_NE(-1, resp.get_body_str().find("X-Eventually"));
    ASSERT_EQ(200, resp.get_code());
}

TEST(http_client, response_headers) {

    http_client client;
    http_request req("http://httpbin.org/get");
    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_headers().size());
    ASSERT_EQ(200, resp.get_code());
    ASSERT_STREQ("application/json", resp.get_header("Content-Type")->second.c_str());
}

TEST(http_client, interrupt) {

    dispatcher d;
    http_client client(d);
    connection c;
    http_request req("http://httpbin.org/get");
    auto f = client.send(c, req);
    c.interrupt();
    d.process_all();

    bool threw = false;
    try
    {
        auto resp = f.get();
    }
    catch(const connection_interrupted&)
    {
        threw = true;
    }
    ASSERT_TRUE(threw);
}

TEST(http_client, when) {

    http_client client;
    http_request req("http://httpbin.org/get");
    auto f = client.get_dispatcher().when([](const http_response& resp){
        return resp.get_body_str().substr(0, 10);
    }, client.send(req));

    ASSERT_STREQ("{\n  \"args\"", f.get().c_str());
}
