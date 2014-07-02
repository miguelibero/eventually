
#include <eventually/dispatcher.hpp>

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
    pet.cancel();
    d.process();
    ASSERT_FALSE(test);
}

