
#ifndef _eventually_then_hpp_
#define _eventually_then_hpp_

#include <future>
#include <iostream>

namespace eventually {

	template <typename T, typename Work>
	auto get_work_done(std::future<T>& f, Work& w) -> decltype(w(f.get()))
	{
		return w(f.get());
	}

	template <typename Work>
	auto get_work_done(std::future<void>& f, Work& w) -> decltype(w())
	{
		f.wait();
		return w();
	}

	template <typename T, typename Work>
	auto then(std::future<T>&& f, Work w) -> std::future<decltype(w(f.get()))>
	{
	    return std::async([](std::future<T>&& f, Work w){
	    	return get_work_done(f,w);
	    }, std::move(f), std::move(w));
	}
}

#endif