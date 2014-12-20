
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
        virtual bool operator()() = 0;
    };

    /**
     * A container for a std::promise and the
     * associated work, handler and connection
     */
    template<class Retry, class Work, class... Args>
    class task : public basic_task
    {
    private:
        typedef typename result_of<Work(Args&&...)>::type result;
        connection _connection;
        Retry _retry;
        Work _work;
        handler<Args...> _handler;
        std::promise<result> _promise;

    public:

        task(connection& c, Retry&& r, Work&& w, Args&&... args):
        _connection(c),
        _retry(std::forward<Retry>(r)),
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

        bool operator()()
        {
            if(!_handler(_retry, _connection))
            {
                return false;
            }
            _handler(_work, _connection, _promise);
            return true;
        }

    };

    /**
     * Helper method to generate tasks
     */
    template <typename Retry, typename Work, typename... Args>
    auto make_task(connection& c, Retry&& r, Work&& w, Args&&... args) -> task<Retry, Work, Args...>
    {
        return task<Retry, Work, Args...>(c, std::forward<Retry>(r), std::forward<Work>(w), std::forward<Args>(args)...);
    }

    /**
     * Helper method to generate task pointers
     */
    template <typename Retry, typename Work, typename... Args>
    auto make_task_ptr(connection& c, Retry&& r, Work&& w, Args&&... args) -> std::unique_ptr<task<Retry, Work, Args...>>
    {
        return std::unique_ptr<task<Retry, Work, Args...>>(
                new task<Retry, Work, Args...>(c, std::forward<Retry>(r), std::forward<Work>(w), std::forward<Args>(args)...));
    }

}

#endif
