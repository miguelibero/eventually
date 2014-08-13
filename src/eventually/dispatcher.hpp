
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task.hpp>
#include <eventually/connection.hpp>
#include <eventually/worker.hpp>
#include <eventually/is_callable.hpp>
#include <eventually/is_same.hpp>
#include <deque>
#include <memory>
#include <mutex>

namespace eventually {

    /**
     * This is a base class for an object that provides std::async like functionality.
     * It stores a list of function objects to be processed some time in the future.
     */
    class dispatcher
    {
    private:
        typedef std::unique_ptr<basic_task> basic_task_ptr;
        std::mutex _mutex;
        std::deque<basic_task_ptr> _tasks;

    protected:
        std::condition_variable _new_task;

    public:

        virtual ~dispatcher();

        template<typename Work, typename... Args, typename std::enable_if<is_callable<Work(Args...)>::value, int>::type = 0>
        auto dispatch(Work&& w, Args&&... args) noexcept ->  std::future<decltype(w(args...))>
        {
            return dispatch(make_task_ptr(std::forward<Work>(w), std::forward<Args>(args)...));
        }

        template<typename Work, typename... Args, typename std::enable_if<is_callable<Work(Args...)>::value, int>::type = 0>
        auto dispatch(connection& c, Work&& w, Args&&... args) noexcept ->  std::future<decltype(w(args...))>
        {
            return dispatch(make_task_ptr(c, std::forward<Work>(w), std::forward<Args>(args)...));
        }

        template<typename Result, typename... Args>
        auto dispatch(std::unique_ptr<task<Result, Args...>> t) noexcept ->  std::future<Result>
        {
            auto future_ = t->get_future();
            {
                std::lock_guard<std::mutex> lock_(_mutex);
                _tasks.push_back(std::move(t));
            }
            _new_task.notify_one();
            return future_;
        }

        /**
         * Call a function when a future is ready.
         * Can be used to concatenate tasks.
         * @param future to wait for
         * @param work function that accepts the future result as a parameter
         * @result future for this task
         */
        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(Work&& w, std::future<Result>&& f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(std::forward<Work>(w), f.share());
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(Work&& w, std::shared_future<Result> f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch([w, f]() mutable {
                return worker::get_work_done(w, f);
            });
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(connection& c, Work&& w, std::future<Result>&& f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(c, std::forward<Work>(w), f.share());
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(connection& c, Work&& w, std::shared_future<Result> f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [w, f]() mutable {
                return worker::get_work_done(w, f);
            });
        }

        template <typename Work, typename... Results, typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(Work&& w, std::future<Results>&&... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return when_all(std::forward<Work>(w), f.share()...);
        }

        template <typename Work, typename... Results, typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(Work&& w, std::shared_future<Results>... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return dispatch([w](std::shared_future<Results>... f) mutable {
                return worker::get_work_done(w, f...);
            }, std::shared_future<Results>(f)...);
        }

        template <typename Work, typename... Results, typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(connection& c, Work&& w, std::future<Results>&&... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return when_all(c, std::forward<Work>(w), f.share()...);
        }

        template <typename Work, typename... Results, typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(connection& c, Work&& w, std::shared_future<Results>... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return dispatch(c, [w](std::shared_future<Results>... f) mutable {
                return worker::get_work_done(w, f...);
            }, std::shared_future<Results>(f)...);
        }

        template <typename... Results>
        auto when_all(std::future<Results>&&... f) noexcept -> std::future<std::tuple<Results...>>
        {
            return when_all(f.share()...);
        }

        template <typename... Results>
        auto when_all(std::shared_future<Results>... f) noexcept -> std::future<std::tuple<Results...>>
        {
            return when_all([](Results... rs){
                return std::tuple<Results...>(rs...);
            }, f...);
        }

        template <typename... Results>
        auto when_all(connection& c, std::future<Results>&&... f) noexcept -> std::future<std::tuple<Results...>>
        {
            return when_all(c, f.share()...);
        }

        template <typename... Results>
        auto when_all(connection& c, std::shared_future<Results>... f) noexcept -> std::future<std::tuple<Results...>>
        {
            return when_all(c, [](Results... rs){
                return std::tuple<Results...>(rs...);
            }, f...);
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when_any(std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) noexcept -> std::future<decltype(w(f.get()))>
        {
            shared_worker<decltype(w(f.get()))> p;
            when_any(std::forward<Work>(w), p, f, fs...);
            return p.get_future();
        }

        template <typename Work, typename FinalResult, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0, typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, shared_worker<FinalResult> p, std::shared_future<Result> f, std::shared_future<Results>... fs) noexcept
        {
            when_any(std::forward<Work>(w), p, fs...);
            when_any(std::forward<Work>(w), p, f);
        }

        template <typename Work, typename FinalResult, typename Result,
            typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, shared_worker<FinalResult> p, std::shared_future<Result> f) noexcept
        {
            dispatch([w, p, f]() mutable {
                return p.get_work_done(w, f);
            });
        }

        bool process_all() noexcept;
        bool process_one() noexcept;

    };
}


#endif
