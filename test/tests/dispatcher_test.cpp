
#include <eventually/dispatcher.hpp>
#include "gtest/gtest.h"

using namespace eventually;

TEST(dispatcher, process_one) {

    dispatcher d;

    auto future1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(future1.valid());

    d.process_one();

    ASSERT_EQ(5, future1.get());
}


TEST(dispatcher, process_all) {

    dispatcher d;

    auto future1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    auto future2 = d.dispatch([](int a, int b){
        return a-b;
    }, 2, 3);

    ASSERT_TRUE(future1.valid());
    ASSERT_TRUE(future2.valid());

    d.process_all();

    ASSERT_EQ(5, future1.get());
    ASSERT_EQ(-1, future2.get());
}
