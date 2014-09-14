
#include <eventually/http_client.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

TEST(http_client, basic) {

    http_client client;
    http_request req("http://httpbin.org/get");

    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_body().size());
    ASSERT_LT(0, resp.get_body_str().find("headers"));
    ASSERT_EQ(200, resp.get_code());
}

TEST(http_client, post) {

	http_client client;
    http_request req("http://httpbin.org/post", http_request_method::POST);
    req.set_body_str("test_eventually");

    auto f = client.send(req);
    auto resp = f.get();

    ASSERT_LT(0, resp.get_body().size());
    ASSERT_LT(0, resp.get_body_str().find("test_eventually"));
    std::cout << resp.get_body_str() << std::endl;
    ASSERT_EQ(200, resp.get_code());

}




