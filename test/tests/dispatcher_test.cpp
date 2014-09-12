#include <eventually/dispatcher.hpp>
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

    auto f2 = d.when([](int c){
        return 2.0f*c ;
    }, std::move(f1));

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
    auto f2 = d.when(std::bind(func2, _1, 2.0f), std::move(f1));

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(18.0f, f2.get());
}

TEST(dispatcher, when_shared) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3).share();

    auto f2 = d.when([](int c){
        return 2.0f*c ;
    }, f1);

    d.process_one();

    ASSERT_EQ(5, f1.get());

    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f2.get());
}

TEST(dispatcher, when_combined) {

    dispatcher d;

    auto f = d.when([](int c){
        return 2.0f*c ;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3));

    d.process_one();
    d.process_one();

    ASSERT_FLOAT_EQ(10.0f, f.get());
}

TEST(dispatcher, when_throw) {

    dispatcher d;

    auto f = d.when([](int c){
        return 2.0f*c ;
    }, d.when_throw([](const std::exception& e){
        return 2;
    }, d.dispatch([](){
        throw std::exception();
        return 0;
    })));

    d.process_one();
    d.process_one();
    d.process_one();
    
    ASSERT_FLOAT_EQ(4.0f, f.get());
}

TEST(dispatcher, when_throw_no_exception) {

    dispatcher d;

    auto f = d.when([](int c){
        return 2.0f*c ;
    }, d.when_throw([](const std::exception& e){
        return 2;
    }, d.dispatch([](){
        return 3;
    })));

    d.process_one();
    d.process_one();
    d.process_one();
    
    ASSERT_FLOAT_EQ(6.0f, f.get());
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

TEST(dispatcher, when_all_cancel) {

    dispatcher d;
    connection c;

    auto f = d.when_all(c, [](int a, float b){
        return a*b;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](float a, float b){
        return a/b;
    }, 4.0f, 2.0f));

    c.interrupt();
    d.process_all();

    bool threw = false;
    try
    {
        f.get();
    }
    catch(const connection_interrupted&)
    {
        threw = true;
    }

    ASSERT_TRUE(threw);
}


TEST(dispatcher, when_all_tuple) {

    dispatcher d;

    auto f = d.when_all(d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](float a, float b){
        return a/b;
    }, 4.0f, 2.0f));

    d.process_all();

    auto t = f.get();

    ASSERT_EQ(5, std::get<0>(t));
    ASSERT_FLOAT_EQ(2.0f, std::get<1>(t));
}

TEST(dispatcher, when_any) {

    dispatcher d;

    auto f = d.when_any([](int a){
        return a;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](int a, int b){
        return a-b;
    }, 3, 2));

    d.process_all();

    ASSERT_EQ(1, f.get());
}

/*
TEST(dispatcher, when_any_void) {

    dispatcher d;
    bool called = false;
    auto f = d.when_any([&called](){
        called = true;
    }, d.dispatch([](){}), d.dispatch([](){}));

    d.process_all();

    ASSERT_TRUE(called);
}
*/

TEST(dispatcher, when_any_cancel) {

    dispatcher d;
    connection c;

    auto f = d.when_any(c, [](int a){
        return a;
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](int a, int b){
        return a-b;
    }, 3, 2));

    c.interrupt();

    d.process_all();

    bool threw = false;
    try
    {
        f.get();
    }
    catch(const connection_interrupted&)
    {
        threw = true;
    }

    ASSERT_TRUE(threw);
}

TEST(dispatcher, when_every) {

    dispatcher d;

    std::vector<int> b;
    auto f = d.when_every([&b](std::vector<int>& a){
        b.insert(b.end(), a.begin(), a.end());
        for(auto& i : a)
        {
            i += 1;
        }
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](int a, int b){
        return a-b;
    }, 3, 2));

    d.process_all();
    auto c = f.get();

    ASSERT_EQ(3, b.size());
    ASSERT_EQ(1, b[0]);    
    ASSERT_EQ(2, b[1]);
    ASSERT_EQ(5, b[2]);
    ASSERT_EQ(2, c.size());
    ASSERT_EQ(3, c[0]);    
    ASSERT_EQ(6, c[1]);
}

TEST(dispatcher, when_every_vector) {

    dispatcher d;

    auto f = d.when_every(d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](int a, int b){
        return a-b;
    }, 3, 2));

    d.process_all();
    auto t = f.get();

    ASSERT_EQ(2, t.size());
    ASSERT_EQ(1, t[0]);    
    ASSERT_EQ(5, t[1]);
}

TEST(dispatcher, when_every_cancel) {

    dispatcher d;
    connection c;

    auto f = d.when_every(c, [](const std::vector<int>& a){
    }, d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3), d.dispatch([](int a, int b){
        return a-b;
    }, 3, 2));

    c.interrupt();
    d.process_all();    
    auto t = f.get();

    ASSERT_EQ(0, t.size());
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
