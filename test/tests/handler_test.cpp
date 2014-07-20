
#include <eventually/handler.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

TEST(handler, basic) {

    handler<int> h([](){
        return 4;
    });

    ASSERT_EQ(4, h());
}

TEST(handler, make) {

    auto h = make_handler([](){
        return 4;
    });

    ASSERT_EQ(4, h());
}


TEST(handler, arguments) {

    handler<int, int, int> h([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_EQ(5, h());
}

TEST(handler, make_arguments) {

    auto h = make_handler([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_EQ(5, h());

}