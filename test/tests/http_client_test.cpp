
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




