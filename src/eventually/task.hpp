
#ifndef _eventually_task_hpp_
#define _eventually_task_hpp_

#include <future>
#include <memory>
#include <eventually/define.hpp>
#include <eventually/connection.hpp>
#include <eventually/handler.hpp>
#include <eventually/is_callable.hpp>
#include <eventually/worker.hpp>

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
     * A container for a std::promise and the
     * associated work, handler and connection
     */
    template<class Work, class... Args>
    class task : public basic_task
    {
    private:
        typedef typename result_of<Work(Args&&...)>::type result;
        connection _connection;
        Work _work;
        handler<Args...> _handler;
        std::promise<result> _promise;

    public:

        task(connection& c, Work&& w, Args&&... args):
        _connection(c),
        _work(std::forward<Work>(w)),
        _handler(std::forward<Args>(args)...)
        {
        }

        std::future<result> get_future()
        {
            return _promise.get_future();
        }

        const connection& get_connection() const
        {
            return _connection;
        }

        connection& get_connection()
        {
            return _connection;
        }

        void operator()()
        {
        }

    };

    /**
     * Helper method to generate tasks
     */
    template <typename Work, typename... Args>
    auto make_task(connection& c, Work&& w, Args&&... args) -> task<Work, Args...>
    {
        return task<Work, Args...>(c, std::forward<Work>(w), std::forward<Args>(args)...);
    }

    /**
     * Helper method to generate task pointers
     */
    template <typename Work, typename... Args>
    auto make_task_ptr(connection& c, Work&& w, Args&&... args) -> std::unique_ptr<task<Work, Args...>>
    {
        return std::unique_ptr<task<Work, Args...>>(
                new task<Work, Args...>(c, std::forward<Work>(w), std::forward<Args>(args)...));
    }

}

#endif
