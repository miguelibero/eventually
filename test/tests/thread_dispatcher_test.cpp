
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

	thread_dispatcher d(std::chrono::milliseconds(10));

    auto future1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(future1.valid());

    ASSERT_EQ(5, future1.get());
}

TEST(thread_dispatcher, when_combined) {

    thread_dispatcher d;

    auto future = d.when([](int c){
        return 2.0f*c ;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3));

    ASSERT_FLOAT_EQ(10.0f, future.get());
}

TEST(thread_dispatcher, connection_interrupt) {

    thread_dispatcher d;
    connection c;
    bool done = false;
    std::atomic<bool> interrupted;
    interrupted.store(false);
    {
        d.dispatch_retry(c, [&interrupted] {
            return interrupted.load();
        }, [c, &done]() mutable {
            c.interruption_point();
            done = true;
        });
        c.interrupt();
        interrupted.store(true);
    }

    ASSERT_FALSE(done);
}

TEST(thread_dispatcher, when_combined_scoped_connection) {

    thread_dispatcher d;
    {
        scoped_connection c;

        d.when(c, [](int c){
            return 2.0f*c;
        }, d.dispatch(c, [](int a, int b){
            return a+b;
        }, 2, 3));
    }
}

/*
TEST(thread_dispatcher, race_condition) {
    int var;

    std::thread([&var](){
        var++;
    });

    std::thread([&var](){
        var++;
    });
}
*/