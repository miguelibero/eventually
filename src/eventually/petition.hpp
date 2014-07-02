
#ifndef _eventually_petition_hpp_
#define _eventually_petition_hpp_

#include <memory>
#include <mutex>

namespace eventually {

	class petition_data;

	class petition
	{
	private:
		std::shared_ptr<petition_data> _data;
	public:
		petition();
		void cancel();
		template<typename F>
		void step(const F& function) const;
		template<typename F>
		void check(const F& function) const;
	};

	class petition_data
	{
	private:
		typedef std::recursive_mutex mutex_t;
		typedef std::lock_guard<mutex_t> lock_guard_t;
		bool _active;
		mutable mutex_t _active_mutex;
	public:
		petition_data();
		void cancel();
		template<typename F>
		void step(const F& function) const;
		template<typename F>
		void check(const F& function) const;
	};

}

#include <eventually/petition.impl.hpp>

#endif