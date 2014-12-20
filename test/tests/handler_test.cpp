
#include <eventually/handler.hpp>
#include <future>
#include "gtest/gtest.h"

using namespace eventually;


TEST(handler, basic) {

    handler<> h;
    connection c;
    std::promise<int> p;
    auto f = p.get_future();

    h([](){
        return 4;
    }, c, p);

    ASSERT_EQ(4, f.get());
}

TEST(handler, make) {

    auto h = make_handler();
    connection c;
    std::promise<int> p;
    auto f = p.get_future();    

    h([](){
        return 4;
    }, c, p);

    ASSERT_EQ(4, f.get());
}

TEST(handler, arguments) {

    handler<int, int> h(2, 3);
    connection c;
    std::promise<int> p;
    auto f = p.get_future();

    h([](int a, int b){
        return a+b;
    }, c, p);    

    ASSERT_EQ(5, f.get());
}

TEST(handler, make_arguments) {

    auto h = make_handler(2, 3);
    connection c;
    std::promise<int> p;
    auto f = p.get_future();

    h([](int a, int b){
        return a+b;
    }, c, p);

    ASSERT_EQ(5, f.get());
}

TEST(handler, unique_ptr) {

    std::unique_ptr<int> ptr(new int(5));
    auto h = make_handler(std::move(ptr));
    connection c;
    std::promise<int> p;
    auto f = p.get_future();

    h([](std::unique_ptr<int> ptr){
        return *ptr;
    }, c, p);

    ASSERT_EQ(5, f.get());
}