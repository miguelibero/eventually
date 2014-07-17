
#include <eventually/thread_dispatcher.hpp>
#include "gtest/gtest.h"

using namespace eventually;

TEST(thread_dispatcher, process) {

    thread_dispatcher d;

    auto future1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(future1.valid());

    ASSERT_EQ(5, future1.get());
}

TEST(thread_dispatcher, process_wait) {

    thread_dispatcher d(thread_dispatcher::duration(0.5));

    auto future1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(future1.valid());

    ASSERT_EQ(5, future1.get());
}



TEST(thread_dispatcher, then_combined) {

    thread_dispatcher d;

    auto future = d.then(d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), [](int c){
        return 2.0f*c ;
    });

    ASSERT_FLOAT_EQ(10.0f, future.get());
}