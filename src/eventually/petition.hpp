
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
		inline petition();
		inline void cancel();
		template<typename F>
		inline void process(const F& function) const;
		template<typename F>
		inline void check(const F& function) const;
	};

	class petition_data
	{
	private:
		bool _active;
		mutable std::recursive_mutex _active_mutex;
	public:
		inline petition_data();
		inline void cancel();
		template<typename F>
		inline void process(const F& function) const;
		template<typename F>
		inline void check(const F& function) const;
	};

}

#include <eventually/petition.impl.hpp>

#endif