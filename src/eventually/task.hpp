
#ifndef _eventually_task_hpp_
#define _eventually_task_hpp_

#include <future>
#include <memory>
#include <eventually/connection.hpp>
#include <eventually/handler.hpp>

namespace eventually {

    /**
     * A basic interface to store tasks of different results
     * in the same dispatcher
     */
    class basic_task
    {
    public:
        virtual ~basic_task();
        virtual void operator()() = 0;
    };

    /**
     * A container for a std::packaged_task and the
     * associated handler and connection
     */
    template<class Result, class... Args>
    class task : public basic_task
    {
    private:
        typedef handler<Result, Args...> internal_handler;
        typedef std::packaged_task<Result()> internal_task;
        connection _connection;
        internal_handler _handler;
        internal_task _task;

    public:

        template<class Work>
        task(Work&& w, Args&&... args):
        _handler(std::forward<Work>(w), std::forward<Args>(args)...),
        _task([this](){ return get_work_done(); })
        {
        }

        template<class Work>
        task(connection& c, Work&& w, Args&&... args):
        _connection(c),
        _handler(std::forward<Work>(w), std::forward<Args>(args)...),
        _task([this](){ return get_work_done(); })
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

    /**
     * Helper method to generate task pointers
     */
    template <typename Work, typename... Args>
    auto make_task_ptr(Work&& w, Args&&... args) -> std::unique_ptr<task<decltype(w(args...)), Args...>>
    {
        return std::unique_ptr<task<decltype(w(args...)), Args...>>(
                new task<decltype(w(args...)), Args...>(std::forward<Work>(w), std::forward<Args>(args)...));
    }

    template <typename Work, typename... Args>
    auto make_task_ptr(connection& c, Work&& w, Args&&... args) -> std::unique_ptr<task<decltype(w(args...)), Args...>>
    {
        return std::unique_ptr<task<decltype(w(args...)), Args...>>(
                new task<decltype(w(args...)), Args...>(c, std::forward<Work>(w), std::forward<Args>(args)...));
    }

}

#endif
