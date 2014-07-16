
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task.hpp>
#include <deque>
#include <type_traits>
#include <memory>
#include <mutex>

namespace eventually {

    class dispatcher
    {
    private:
    	typedef std::unique_ptr<basic_task> basic_task_ptr;
    	std::mutex _mutex;
    	std::deque<basic_task_ptr> _tasks;

    public:

		template<class Function, class... Args>
        std::future<typename std::result_of<Function(Args...)>::type> dispatch(Function&& f, Args&&... args)
        {
        	std::unique_ptr<task<typename std::result_of<Function(Args...)>::type>> task_(
        		new task<typename std::result_of<Function(Args...)>::type>(f, args...));
        	auto future_ = task_->get_future();
        	std::lock_guard<std::mutex> lock_(_mutex);
        	_tasks.push_back(std::move(task_));
        	return future_;
        }

        bool process_all();
        bool process_one();


    };
}


#endif