
#ifndef _eventually_task_hpp_
#define _eventually_task_hpp_

#include <future>
#include <functional>
#include <eventually/connection.hpp>

namespace eventually {

    /**
     * A basic interface to store tasks of different results
     * in the same dispatcher.
     */
    class basic_task
    {
    public:
        virtual ~basic_task();
        virtual void operator()() = 0;
    };

    /**
     * A container for a std::packaged_task and the associated connection
     */
    template<class Result>
    class task : public basic_task
    {
    private:
        typedef std::packaged_task<Result()> internal_task;
        typedef std::function<Result()> handler;
        connection _connection;        
        handler _handler;
        internal_task _task;

    public:

        template<class Work, class... Args>
        task(Work&& w, Args&&... args):
        _handler(std::bind(w, args...)),
        _task(std::bind(&task::get_work_done, this))
        {
        }

        template<class Work, class... Args>
        task(connection& c, Work w, Args... args):
        _connection(c), _handler(std::bind(w, args...)),
        _task(std::bind(&task::get_work_done, this))
        {
        }
         
        std::future<Result> get_future()
        {
            return _task.get_future();
        }

        const connection& get_connection() const
        {
            return _connection;
        }

        connection& get_connection()
        {
            return _connection;
        }

        Result get_work_done()
        {
            return _connection.get_work_done(_handler);
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