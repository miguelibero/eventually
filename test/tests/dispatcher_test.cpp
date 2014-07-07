
#include <eventually/dispatcher.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

TEST(dispatcher, process_single) {

    dispatcher<std::function<void()>> d;

    bool test = false;
    auto pet = d.dispatch([&test](){
        test = true;
    });

    d.process();
    ASSERT_TRUE(test);

    test = false;
    pet = d.dispatch([&test](){
        test = true;
    });
    pet.close();
    d.process();
    ASSERT_FALSE(test);

}


TEST(dispatcher, process_stepped) {

    dispatcher<std::function<void()>> d;

    bool test = false;
    petition pet;

    pet >> d.dispatch([&d, &test, pet](){

        pet >> d.dispatch([&test, pet](){
            test = true;
        });

    });

    d.process();
    ASSERT_FALSE(test);
    d.process();
    ASSERT_TRUE(test);

}


TEST(dispatcher, process_stepped_cancel) {

    dispatcher<std::function<void()>> d;

    bool test = false;
    petition pet;

    pet >> d.dispatch([&d, &test, pet](){

        pet >> d.dispatch([&test, pet](){
            test = true;
        });

    });

    d.process();
    ASSERT_FALSE(test);

    pet.close();

    d.process();
    ASSERT_FALSE(test);

}