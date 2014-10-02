#include <eventually/apply.hpp>
#include <memory>
#include "gtest/gtest.h"

using namespace eventually;

TEST(apply, basic) {

	auto r = apply([](int a, float b){
		return b*a;
	}, std::make_tuple(3, 2.5));

	ASSERT_FLOAT_EQ(7.5, r);
}


TEST(apply, unique_ptr_rvalue) {

	std::unique_ptr<int> p(new int(5));

	auto r = apply([](std::unique_ptr<int>&& ptr){
		return *ptr;
	}, std::make_tuple(std::move(p)));

	ASSERT_EQ(5, r);
}

TEST(apply, unique_ptr_lvalue) {

	std::unique_ptr<int> p(new int(5));

	auto r = apply([](std::unique_ptr<int> ptr){
		return *ptr;
	}, std::make_tuple(std::move(p)));

	ASSERT_EQ(5, r);
}

TEST(apply, unique_ptr_const_ref) {

	std::unique_ptr<int> p(new int(5));

	auto r = apply([](const std::unique_ptr<int>& ptr){
		return *ptr;
	}, std::make_tuple(std::move(p)));

	ASSERT_EQ(5, r);
}