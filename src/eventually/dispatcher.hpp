
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
#include <vector>

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
        auto dispatch(Work&& w, Args&&... args) NOEXCEPT ->  std::future<decltype(w(args...))>
        {
            return dispatch(make_task_ptr(std::forward<Work>(w), std::forward<Args>(args)...));
        }

        template<typename Work, typename... Args, typename std::enable_if<is_callable<Work(Args...)>::value, int>::type = 0>
        auto dispatch(connection& c, Work&& w, Args&&... args) NOEXCEPT ->  std::future<decltype(w(args...))>
        {
            return dispatch(make_task_ptr(c, std::forward<Work>(w), std::forward<Args>(args)...));
        }

        template<typename Result, typename... Args>
        auto dispatch(std::unique_ptr<task<Result, Args...>> t) NOEXCEPT ->  std::future<Result>
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
         * @param work function that accepts the future result as a parameter
         * @param future to wait for         
         * @result future for this task
         */
        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return dispatch([w](std::future<Result> f) mutable {
                return when_worker::work(w, std::move(f));
            }, std::move(f));
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(Work&& w, std::shared_future<Result> f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return dispatch([w, f]() mutable {
                return when_worker::work(w, f);
            });
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(connection& c, Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return when(c, std::forward<Work>(w), f.share());
        }

        template <typename Work, typename Result, typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when(connection& c, Work&& w, std::shared_future<Result> f) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [w, f]() mutable {
                return when_worker::work(w, f);
            });
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
            return when_throw(std::forward<Work>(w), f.share());
        }

        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        auto when_throw(Work&& w, std::shared_future<Result> f) NOEXCEPT -> std::future<Result>
        {
            return dispatch([w, f]() mutable {
                return when_throw_worker::work(w, f);
            });
        }

        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        auto when_throw(connection& c, Work&& w, std::future<Result>&& f) NOEXCEPT -> std::future<Result>
        {
            return when_throw(c, std::forward<Work>(w), f.share());
        }

        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        auto when_throw(connection& c, Work&& w, std::shared_future<Result> f) NOEXCEPT -> std::future<Result>
        {
            return dispatch(c, [w, f]() mutable {
                return when_throw_worker::work(w, f);
            });
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
            return when_all(std::forward<Work>(w), f.share()...);
        }

        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(Work&& w, std::shared_future<Results>... f) NOEXCEPT -> std::future<decltype(w(f.get()...))>
        {
            return dispatch([w](std::shared_future<Results>... f) mutable {
                return when_worker::work(w, f...);
            }, std::shared_future<Results>(f)...);
        }

        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(connection& c, Work&& w, std::future<Results>&&... f) NOEXCEPT -> std::future<decltype(w(f.get()...))>
        {
            return when_all(c, std::forward<Work>(w), f.share()...);
        }

        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        auto when_all(connection& c, Work&& w, std::shared_future<Results>... f) NOEXCEPT -> std::future<decltype(w(f.get()...))>
        {
            return dispatch(c, [w](std::shared_future<Results>... f) mutable {
                return when_worker::work(w, f...);
            }, std::shared_future<Results>(f)...);
        }

        template <typename... Results>
        auto when_all(std::future<Results>&&... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            return when_all(f.share()...);
        }

        template <typename... Results>
        auto when_all(std::shared_future<Results>... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            return when_all([](Results... rs){
                return std::tuple<Results...>(rs...);
            }, f...);
        }

        template <typename... Results>
        auto when_all(connection& c, std::future<Results>&&... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            return when_all(c, f.share()...);
        }

        template <typename... Results>
        auto when_all(connection& c, std::shared_future<Results>... f) NOEXCEPT -> std::future<std::tuple<Results...>>
        {
            return when_all(c, [](Results... rs){
                return std::tuple<Results...>(rs...);
            }, f...);
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
            return when_any(std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            when_any_worker<decltype(w(f.get()))> p(sizeof...(Results)+1);
            when_any(std::forward<Work>(w), p, f, fs...);
            return p.get_future();
        }

        template <typename Work, typename FinalResult, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT
        {
            when_any(std::forward<Work>(w), p, fs...);
            when_any(std::forward<Work>(w), p, f);
        }

        template <typename Work, typename FinalResult, typename Result,
            typename std::enable_if<is_callable_with_result<Work(Result), FinalResult>::value, int>::type = 0>
        void when_any(Work&& w, when_any_worker<FinalResult> p, std::shared_future<Result> f) NOEXCEPT
        {
            dispatch([w, p, f]() mutable {
                return p.work(w, f);
            });
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(connection& c, Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            return when_any(c, std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <typename Work, typename Result, typename... Results,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Result)>::value, int>::type = 0>
        auto when_any(connection& c, Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT -> std::future<decltype(w(f.get()))>
        {
            when_any_worker<decltype(w(f.get()))> p(sizeof...(Results)+1, c);
            when_any(std::forward<Work>(w), p, f, fs...);
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
            typename Container = std::vector<Result>,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        auto when_every(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<Container>
        {
            return when_every(std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <typename Work, typename Result, typename... Results,
            typename Container = std::vector<Result>,        
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        auto when_every(Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT -> std::future<Container>
        {
            when_every_worker<Result> p(sizeof...(Results)+1);
            when_every(std::forward<Work>(w), p, f, fs...);
            return p.get_future();
        }

        template <typename Work, typename Result, typename... Results,
            typename Container = std::vector<Result>,        
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        void when_every(Work&& w, when_every_worker<Result> p, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT
        {
            when_every(std::forward<Work>(w), p, fs...);
            when_every(std::forward<Work>(w), p, f);
        }

        template <typename Work, typename Result,
            typename Container = std::vector<Result>,        
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        void when_every(Work&& w, when_every_worker<Result> p, std::shared_future<Result> f) NOEXCEPT
        {
            dispatch([w, p, f]() mutable {
                return p.work(w, f);
            });
        }

        template <typename Work, typename Result, typename... Results,
            typename Container = std::vector<Result>,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        auto when_every(connection& c, Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<Container>
        {
            return when_every(c, std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <typename Work, typename Result, typename... Results,
            typename Container = std::vector<Result>,        
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        auto when_every(connection& c, Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) NOEXCEPT -> std::future<Container>
        {
            when_every_worker<Result> p(sizeof...(Results)+1, c);
            when_every(std::forward<Work>(w), p, f, fs...);
            return p.get_future();
        }


        template <typename Result, typename... Results,
            typename Container = std::vector<Result>,
            typename std::enable_if<is_same<Result, Results...>::value, int>::type = 0>
        auto when_every(std::future<Result>&& f, std::future<Results>&&... fs) NOEXCEPT -> std::future<Container>
        {
            return when_every([](const Container&){}, f.share(), fs.share()...);
        }

        bool process_all() NOEXCEPT;
        bool process_one() NOEXCEPT;

    };
}


#endif
