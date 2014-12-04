
#include <eventually/handler.hpp>
#include <functional>
#include <memory>
#include "gtest/gtest.h"

using namespace eventually;


TEST(handler, basic) {

    handler<> h;

    ASSERT_EQ(4, h([](){
        return 4;
    }));
}

TEST(handler, make) {

    auto h = make_handler();

    ASSERT_EQ(4, h([](){
        return 4;
    }));
}

TEST(handler, arguments) {

    handler<int, int> h(2, 3);

    ASSERT_EQ(5, h([](int a, int b){
        return a+b;
    }));
}

TEST(handler, make_arguments) {

    auto h = make_handler(2, 3);

    ASSERT_EQ(5, h([](int a, int b){
        return a+b;
    }));
}


TEST(handler, unique_ptr) {

    std::unique_ptr<int> p(new int(5));

    auto h = make_handler(std::move(p));

    ASSERT_EQ(5, h([](std::unique_ptr<int> ptr){
        return *ptr;
    }));
}