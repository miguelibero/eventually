
#include <eventually/dispatcher.hpp>
#include <eventually/then.hpp>
#include "gtest/gtest.h"

using namespace eventually;

TEST(then, process) {

    dispatcher d;

    auto f1 = d.dispatch([](int a, int b){
        return a+b;
    }, 2, 3);

    int value = 0;
    auto f2 = then(std::move(f1), [&value](int sum){
    	value = sum;
    });

    ASSERT_EQ(0, value);

    d.process_one();
    f2.wait();

    ASSERT_EQ(5, value);
}
