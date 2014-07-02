
#include <eventually/petition.hpp>

#include "gtest/gtest.h"

using namespace eventually;

TEST(petition, step_single) {

	petition p;

	bool test = false;
	p.step([&test](){
		test = true;
	});
	ASSERT_TRUE(test);

	p.cancel();
	test = false;
	p.step([&test](){
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

	p.cancel();
	test = false;
	p.check([&test](bool active){
		test = active;
	});

	ASSERT_FALSE(test);
}


