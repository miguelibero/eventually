
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
