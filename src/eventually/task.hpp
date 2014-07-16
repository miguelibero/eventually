
#ifndef _eventually_task_hpp_
#define _eventually_task_hpp_

#include <future>
#include <functional>

namespace eventually {

	class basic_task
	{
	public:
		virtual ~basic_task();
		virtual void operator()() = 0;
	};


 	template<class Result>
	class task : public basic_task
	{
	private:
		typedef std::packaged_task<Result()> internal_task;
		internal_task _task;

	public:

		template<class Function, class... Args>
		task(Function&& f, Args&&... args):
		_task(std::bind(f, args...))
		{
		}
		 
		std::future<Result> get_future()
		{
			return _task.get_future();
		}
		 
		void operator()()
		{
			_task();
		}

		bool valid() const noexcept
		{
			return _task.valid();
		}

		void swap( task& other ) noexcept
		{
			_task.swap(other._task);
		}
		 
		void make_ready_at_thread_exit()
		{
			_task.make_ready_at_thread_exit();
		}
		 
		void reset()
		{
			_task.reset();
		}
	};
}

#endif