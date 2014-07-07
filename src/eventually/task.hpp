
#ifndef _eventually_task_hpp_
#define _eventually_task_hpp_

#include <functional>
#include <eventually/petition.hpp>

namespace eventually {

	class task
	{
	public:
		typedef std::function<void()> handler;
	private:
		handler _handler;
		petition _petition;

	public:
		task(const handler& handler=nullptr);
		petition& get_petition();
		void operator()();
	};
}

#endif