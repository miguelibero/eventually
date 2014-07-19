#include <eventually/eventually.hpp>
#include "gtest/gtest.h"

using namespace eventually;
using namespace std::placeholders;

TEST(dispatcher, process_one) {

    dispatcher d;

    auto f = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    ASSERT_TRUE(f.valid());

    d.process_one();

    ASSERT_EQ(5, f.get());
}


TEST(dispatcher, bind) {

    dispatcher d;

    auto func = [](int a, int b, int c){
        return a+b+c;
    };

    auto f = d.dispatch(std::bind(func, _1, _2, 4), 2, 3);

    ASSERT_TRUE(f.valid());

    d.process_one();

    ASSERT_EQ(9, f.get());
}

TEST(dispatcher, when) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    auto f2 = d.when(std::move(f1), [](int c){
        return 2.0f*c ;
    });

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f2.get());
}

TEST(dispatcher, when_bind) {

    dispatcher d;

    auto func1 = [](int a, int b, int c){
        return a+b+c;
    };

    auto func2 = [](int c, float f){
        return f*c ;
    };

    auto f1 = d.dispatch(std::bind(func1, _1, _2, 4), 2, 3);
    auto f2 = d.when(std::move(f1), std::bind(func2, _1, 2.0f));

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(18.0f, f2.get());
}

TEST(dispatcher, when_shared) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3).share();

    auto f2 = d.when(f1, [](int c){
        return 2.0f*c ;
    });

    d.process_one();

    ASSERT_EQ(5, f1.get());

    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f2.get());
}

TEST(dispatcher, when_combined) {

    dispatcher d;

    auto f = d.when(d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), [](int c){
        return 2.0f*c ;
    });

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f.get());
}

TEST(dispatcher, when_all) {

    dispatcher d;

    auto f = d.when_all([](int a, float b){
        return a*b;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](float a, float b){
        return a/b;
    }, 4.0f, 2.0f));

    d.process_all();

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