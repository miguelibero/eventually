
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

    thread_dispatcher d(thread_dispatcher::duration(0.01));

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

TEST(dispatcher, connection_interrupt) {

    thread_dispatcher d;
    connection c;

    bool done = false;
    d.dispatch(c, [c, &done]() mutable {
        std::this_thread::sleep_for(std::chrono::duration<float>(0.01f));
        c.interruption_point();
        done = true;
    });
    c.interrupt();

    ASSERT_FALSE(done);
}

TEST(thread_dispatcher, then_combined_scoped_connection) {

    thread_dispatcher d;
    {
        scoped_connection c;

        d.then(c, d.dispatch(c, [](int a, int b){
            return a+b;
        }, 2, 3), [](int c){
            return 2.0f*c;
        });
    }
}