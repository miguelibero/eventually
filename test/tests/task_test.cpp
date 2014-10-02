
#include <eventually/task.hpp>
#include <functional>
#include <memory>
#include "gtest/gtest.h"

using namespace eventually;

TEST(task, basic) {

    task<int> t([](){
        return 4;
    });

    t();

    ASSERT_EQ(4, t.get_future().get());
}

TEST(task, make_task_ptr) {

    connection c;
    auto t = make_task_ptr(c, [](int a, int b){
        return a+b;
    }, 2, 3);

    (*t)();

    ASSERT_EQ(5, t->get_future().get());
}

TEST(task, interrupt) {

    connection c;
    task<int> t(c, [](){
        return 4;
    });

    c.interrupt();
    t();

    bool interrupted = false;
    try
    {
        t.get_future().get();
    }
    catch(connection_interrupted&)
    {
        interrupted = true;
    }

    ASSERT_TRUE(interrupted);
}

TEST(task, unique_ptr) {

    std::unique_ptr<int> p(new int(5));
    connection c;
    auto t = make_task_ptr(c, [](std::unique_ptr<int> p){
        return *p;
    }, std::move(p));

    (*t)();

    ASSERT_EQ(5, t->get_future().get());
}