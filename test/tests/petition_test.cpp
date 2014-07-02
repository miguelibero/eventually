
#include <eventually/petition.hpp>
#include <thread>
#include <vector>
#include "gtest/gtest.h"

using namespace eventually;

TEST(petition, process_single) {

    petition p;

    bool test = false;
    p.process([&test](){
        test = true;
    });
    ASSERT_TRUE(test);

    p.close();
    test = false;
    p.process([&test](){
        test = true;
    });

    ASSERT_FALSE(test);
}

TEST(petition, check_single) {

    petition p;

    bool test = false;
    p.check([&test](bool active){
        test = active;
    });
    ASSERT_TRUE(test);

    p.close();
    test = false;
    p.check([&test](bool active){
        test = active;
    });

    ASSERT_FALSE(test);
}

TEST(petition, copy) {

    petition p1;
    petition p2(p1);

    p1.close();

    bool test = true;
    p2.check([&test](bool active){
        test = active;
    });
    ASSERT_FALSE(test);

}

TEST(petition, multithread) {

    petition p;

    int test = 0;
    std::vector<std::thread> threads;
    for(int i=0; i<1000; i++)
    {
        threads.push_back(std::thread([&test, p]() mutable {
            p.process<std::function<void()>>([&test, p]() mutable {
                test++;
                p.close();
            });
        }));
    }
    ASSERT_EQ(1, test);
    for(auto& thread : threads)
    {
        thread.join();
    }
}
