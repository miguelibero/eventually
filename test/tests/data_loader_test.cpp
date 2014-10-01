
#include <eventually/data_loader.hpp>
#include <eventually/file_data_loader.hpp>
#include <eventually/setup_data_loader.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/connection.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;

TEST(data_loader, file) {

	file_data_loader loader;

	auto f = loader.load("README.md");
	auto data = f.get();
	ASSERT_LT(0, data->size());

	std::string str(data->begin(), data->end());
	ASSERT_NE(std::string::npos, str.find("Anthony Williams"));
}

TEST(data_loader, setup) {

	file_data_loader loader;
	setup_data_loader<file_data_loader> sloader(loader);

	sloader.set_name_setup([](std::string& name){
		name += ".md";
	});
	sloader.set_data_setup([](data& data){
		data.clear();
		data.push_back(10);
	});
	auto f = sloader.load("README");
	auto data = f.get();
	ASSERT_LT(1, data->size());
	ASSERT_EQ(10, data->at(1));
}