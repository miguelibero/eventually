
#include <eventually/dispatcher.hpp>
#include "gtest/gtest.h"

using namespace eventually;

TEST(dispatcher, process_one) {

    dispatcher d;

    auto f = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(f.valid());

    d.process_one();

    ASSERT_EQ(5, f.get());
}

TEST(dispatcher, then) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    auto f2 = d.then(std::move(f1), [](int c){
        return 2.0f*c ;
    });

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f2.get());
}

TEST(dispatcher, then_shared) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3).share();

    auto f2 = d.then(f1, [](int c){
        return 2.0f*c ;
    });

    d.process_one();

    ASSERT_EQ(5, f1.get());

    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f2.get());
}

TEST(dispatcher, then_combined) {

    dispatcher d;

    auto f = d.then(d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), [](int c){
        return 2.0f*c ;
    });

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f.get());
}

TEST(dispatcher, connection) {

    dispatcher d;
    connection c;

    bool done = false;
    d.dispatch(c, [&done](){
        done = true;
    });

    c.interrupt();

    d.process_all();

    ASSERT_FALSE(done);
}

TEST(dispatcher, connection_interrupted_exception) {

    dispatcher d;
    connection c;

    bool done = false;
    auto f = d.dispatch(c, [&done](){
        done = true;
    });

    c.interrupt();

    d.process_all();

    bool interrupted = false;
    try
    {
        f.get();
    }
    catch(connection_interrupted&)
    {
        interrupted = true;
    }

    ASSERT_TRUE(interrupted);
}

TEST(dispatcher, process_all) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    auto f2 = d.dispatch([](int a, int b){
        return a-b;
    }, 2, 3);

    ASSERT_TRUE(f1.valid());
    ASSERT_TRUE(f2.valid());

    d.process_all();

    ASSERT_EQ(5, f1.get());
    ASSERT_EQ(-1, f2.get());
}
