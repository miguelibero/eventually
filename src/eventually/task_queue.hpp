
#ifndef _eventually_task_queue_hpp_
#define _eventually_task_queue_hpp_

#include <eventually/task.hpp>
#include <deque>
#include <mutex>

namespace eventually {

    /**
     * taken from the book Concurrency in action by Anthony Williams
     * http://www.cplusplusconcurrencyinaction.com/
     */
    class task_queue
    {
    private:
        std::deque<task> _queue;
        mutable std::mutex _mutex;
        
    public:
        task_queue();
        task_queue(const task_queue& other)=delete;
        task_queue& operator=(const task_queue& other)=delete;

        petition push_front(task data);
        petition push_back(task data);
        bool empty() const;
        bool try_pop_front(task& res);
        bool try_pop_back(task& res);
    };
}

#endif