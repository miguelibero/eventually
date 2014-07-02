
#include <eventually/petition.hpp>

#include "gtest/gtest.h"

using namespace eventually;

TEST(petition, process_single) {

	petition p;

	bool test = false;
	p.process([&test](){
		test = true;
	});
	ASSERT_TRUE(test);

	p.cancel();
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

	p.cancel();
	test = false;
	p.check([&test](bool active){
		test = active;
	});

	ASSERT_FALSE(test);
}

TEST(petition, copy) {

	petition p1;
	petition p2(p1);

	p1.cancel();

	bool test = true;
	p2.check([&test](bool active){
		test = active;
	});
	ASSERT_FALSE(test);

}

TEST(petition, multithread) {


}
