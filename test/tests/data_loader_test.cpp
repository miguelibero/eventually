
#include <eventually/data_loader.hpp>
#include <eventually/file_data_loader.hpp>
#include <eventually/setup_data_loader.hpp>
#include <eventually/http_data_loader.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/connection.hpp>
#include <functional>
#include "gtest/gtest.h"

using namespace eventually;


TEST(data_loader, file) {

	file_data_loader loader;

	auto f = loader.load("README.md");
	auto data = f.get();
	ASSERT_LT(0, (int)data->size());

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
	ASSERT_EQ(1, (int)data->size());
	ASSERT_EQ(10, data->at(0));
}

TEST(data_loader, http) {

	http_data_loader loader;
	setup_data_loader<http_data_loader> sloader(loader);

	sloader.set_name_setup([](std::string& name){
		name = std::string("http://httpbin.org/")+name;
	});

	auto f = sloader.load("get");
	auto data = f.get();
	ASSERT_LT(0, (int)data->size());
	ASSERT_EQ('{', data->at(0));
}


TEST(data_loader, README) {

	std::atomic<size_t> size;
	size.store(-1);
	file_data_loader loader;
	connection conn;

	std::atomic_bool block;
	block.store(true);
	loader.get_dispatcher().when(conn, [&block,&size](std::unique_ptr<std::vector<uint8_t>>&& data){
	    block.store(false);
	    size.store(data->size());
	}, loader.load(conn, "/etc/magic"));
	while(block.load()){};

	ASSERT_LT((size_t)0, size.load());
}