
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/define.hpp>
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

        /**
         * Do work in the future
         * @param connection that is used to interrupt the work
         * @param work function
         * @param args additional arguments
         */
        template<typename Work, typename... Args, typename std::enable_if<is_callable<Work(Args&&...)>::value, int>::type = 0>
        auto dispatch(Work&& w, Args&&... args) NOEXCEPT -> std::future<decltype(w(std::forward<Args>(args)...))>
        {
            connection c;
            return dispatch(c, std::forward<Work>(w), std::forward<Args>(args)...);
        }

        template<typename Work, typename... Args, typename std::enable_if<is_callable<Work(Args&&...)>::value, int>::type = 0>
        auto dispatch(connection& c, Work&& w, Args&&... args) NOEXCEPT -> std::future<decltype(w(std::forward<Args>(args)...))>
        {
            auto t = make_task_ptr(c, std::forward<Work>(w), std::forward<Args>(args)...);
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
         * @param work function that accepts the future result as a parameter
         * @param future to wait for         
         * @result future for this task
         */
        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            connection c;
            return when(c, std::forward<Work>(w), std::move(f));
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(connection& c, Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [w](std::future<Result>&& f) mutable {
                return when_worker::work(w, f);
            }, std::move(f));
        }

        /**
         * Call a function when a future throws an exception
         * Can be used to react to asyncronous exception
         * @param work function that accepts the exception as a parameter         
         * @param future to wait for
         * @result future for this task
         */
        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        auto when_throw(Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<Result>
        {
            connection c;
            return when_throw(c, std::forward<Work>(w), std::move(f));
        }

        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        auto when_throw(connection& c, Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<Result>
        {
            return dispatch(c, [w](std::future<Result>&& f) mutable {
                return when_throw_worker::work(w, f);
            }, std::move(f));
        }

        /**
         * Call a function when a a list of futures are met
         * @param work function that accepts results as parameters         
         * @param futures to wait for
         * @result future for this task
         */
        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(Work&& w, std::future<Results>&&... f) NOEXCEPT -> std::future<decltype(w(f.get()...))>
        {
            connection c;
            return when_all(c, std::forward<Work>(w), std::move(f)...);
        }

        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(connection& c, Work&& w, std::future<Results>&&... f) NOEXCEPT -> std::future<decltype(w(f.get()...))>
        {
            return dispatch(c, [w](std::future<Results>&&... f) mutable {
                return when_worker::work(w, f...);
            }, std::move(f)...);
        }

        template <typename... Results>
        auto when_all(std::future<Results>&&... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            connection c;
            return when_all(c, std::move(f)...);
        }

        template <typename... Results>
        auto when_all(connection& c, std::future<Results>&&... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            return when_all(c, [](Results... rs){
                return std::tuple<Results...>(rs...);
            }, std::move(f)...);
        }

        /**
         * Call a function when a the first future of a list is met
         * Only works with a list of futures of the same type
         * @param work function that accepts a result as parameter
         * @param futures to wait for
         * @result future for this task
         */
        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            connection c;
            return when_any(c, std::forward<Work>(w), std::move(f), std::move(fs)...);
        }

        template <typename Work, typename FinalResult, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT
        {
            when_any(std::forward<Work>(w), p, std::move(fs)...);
            when_any(std::forward<Work>(w), p, std::move(f));
        }

        template <typename Work, typename FinalResult, typename Result,
            typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::future<Result> f) NOEXCEPT
        {
            dispatch([w, p](std::future<Result>&& f) mutable {
                return p.work(w, f);
            }, std::move(f));
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(connection& c, Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            when_any_worker<decltype(w(f.get()))> p(sizeof...(Results)+1, c);
            when_any(std::forward<Work>(w), p, std::move(f), std::move(fs)...);
            return p.get_future();
        }

        // special cases for void futures
        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, void>::value, int>::type = 0,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work()>::value, int>::type = 0>
        auto when_any(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<decltype(w())>
        {
            connection c;
            return when_any(c, std::forward<Work>(w), std::move(f), std::move(fs)...);
        }

        template <typename Work, typename FinalResult, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, void>::value, int>::type = 0,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable_with_result<Work(), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT
        {
            when_any(std::forward<Work>(w), p, std::move(fs)...);
            when_any(std::forward<Work>(w), p, std::move(f));
        }

        template <typename Work, typename FinalResult, typename Result,
            typename std::enable_if<is_same<Result, void>::value, int>::type = 0,
            typename std::enable_if<is_callable_with_result<Work(), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::future<Result> f) NOEXCEPT
        {
            dispatch([w, p](std::future<Result>&& f) mutable {
                return p.work(w, f);
            }, std::move(f));
        }        

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, void>::value, int>::type = 0,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work()>::value, int>::type = 0>
        auto when_any(connection& c, Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<decltype(w())>
        {
            when_any_worker<decltype(w())> p(sizeof...(Results)+1, c);
            when_any(std::forward<Work>(w), p, std::move(f), std::move(fs)...);
            return p.get_future();
        }

        /**
         * Call a function when every future of a list is met
         * Only works with a list of futures of the same type
         * @param work function that accepts a container of results as parameter
         * @param futures to wait for
         * @result future for this task
         */
        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(when_every_container<Result>&)>::value, int>::type = 0>
        auto when_every(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> when_every_future<Result>
        {
            connection c;
            return when_every(c, std::forward<Work>(w), std::move(f), std::move(fs)...);
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(when_every_container<Result>&)>::value, int>::type = 0>
        void when_every(Work&& w, when_every_worker<Result> p, std::future<Result> f, std::future<Results>... fs) NOEXCEPT
        {
            when_every(std::forward<Work>(w), p, std::move(fs)...);
            when_every(std::forward<Work>(w), p, std::move(f));
        }

        template <typename Work, typename Result,
            typename std::enable_if<is_callable<Work(when_every_container<Result>&)>::value, int>::type = 0>
        void when_every(Work&& w, when_every_worker<Result> p, std::future<Result> f) NOEXCEPT
        {
            dispatch([w, p](std::future<Result>&& f) mutable {
                return p.work(w, f);
            }, std::move(f));
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(when_every_container<Result>&)>::value, int>::type = 0>
        auto when_every(connection& c, Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> when_every_future<Result>
        {
            when_every_worker<Result> p(sizeof...(Results)+1, c);
            when_every(std::forward<Work>(w), p, std::move(f), std::move(fs)...);
            return p.get_future();
        }


        template <typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0>
        auto when_every(std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> when_every_future<Result>
        {
            return when_every([](const when_every_container<Result>&){}, std::move(f), std::move(fs)...);
        }

        bool process_all() NOEXCEPT;
        bool process_one() NOEXCEPT;

    };
}


#endif
