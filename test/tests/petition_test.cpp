
#include <eventually/petition.hpp>
#include <thread>
#include <vector>
#include "gtest/gtest.h"

using namespace eventually;

TEST(petition, active) {

    petition p;

    ASSERT_TRUE(p.active());

    p.close();

    ASSERT_FALSE(p.active());
}

TEST(petition, copy_ctor) {

    petition p1;
    petition p2(p1);

    p1.close();

    ASSERT_FALSE(p1.active());
    ASSERT_FALSE(p2.active());
}

TEST(petition, copy_left) {

    petition p1;
    petition p2;
    petition p3;

    p3 = p2;
    p2 = p1;

    p1.close();

    ASSERT_FALSE(p1.active());
    ASSERT_FALSE(p2.active());
    ASSERT_FALSE(p3.active());
}


TEST(petition, copy_right) {

    petition p1;
    petition p2;
    petition p3;

    p2 >> p3;
    p1 >> p2;

    p1.close();

    ASSERT_FALSE(p1.active());
    ASSERT_FALSE(p2.active());
    ASSERT_FALSE(p3.active());
}

TEST(petition, multithread) {

    petition p;

    int test = 0;
    std::vector<std::thread> threads;
    for(int i=0; i<1000; i++)
    {
        threads.push_back(std::thread([&test, p]() mutable {
            test += p.close() ? 1 : 0;
        }));
    }
    ASSERT_EQ(1, test);
    for(auto& thread : threads)
    {
        thread.join();
    }
}
