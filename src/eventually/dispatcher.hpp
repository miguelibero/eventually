
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task.hpp>
#include <eventually/connection.hpp>
#include <functional>
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

        template <typename Result, typename Work>
        static auto get_work_done(std::shared_future<Result>& f, Work& w) -> decltype(w(f.get()))
        {
            return w(f.get());
        }

        
        template <typename Work>
        static auto get_work_done(std::shared_future<void>& f, Work& w) -> decltype(w())
        {
            f.wait();
            return w();
        }

    public:

        template<class Work, class... Args>
        auto dispatch(Work&& w, Args&&... args) noexcept ->  std::future<decltype(w(args...))>
        {
            auto task_ = make_task_ptr(std::forward<Work>(w), std::forward<Args>(args)...);
            auto future_ = task_->get_future();
            std::lock_guard<std::mutex> lock_(_mutex);
            _tasks.push_back(std::move(task_));
            return future_;
        }

        template<class Work, class... Args>
        auto dispatch(connection& c, Work&& w, Args&&... args) noexcept ->  std::future<decltype(w(args...))>
        {
            auto task_ = make_task_ptr(c, std::forward<Work>(w), std::forward<Args>(args)...);
            auto future_ = task_->get_future();
            std::lock_guard<std::mutex> lock_(_mutex);
            _tasks.push_back(std::move(task_));
            return future_;
        }

        /**
         * Call a function when a future is ready.
         * Can be used to concatenate tasks.
         * @param future to wait for
         * @param work function
         * @result future for this task
         */
        template <class Work, class Result>
        auto when(std::future<Result>&& f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(f.share(), std::forward<Work>(w));
        }

        template <class Work, class Result>
        auto when(std::shared_future<Result> f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch([f, w]() mutable {
                return get_work_done(f, w);
            });
        }

        template <class Work, class Result>
        auto when(connection& c, std::future<Result>&& f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(c, f.share(), std::forward<Work>(w));
        }

        template <class Work, class Result>
        auto when(connection& c, std::shared_future<Result> f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [f, w]() mutable {
                return get_work_done(f, w);
            });
        }

        template <class Work, class... Results>
        auto when_all(Work&& w, std::future<Results>&&... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return when_all(std::forward<Work>(w), f.share()...);
        }

        template <class Work, class... Results>
        auto when_all(Work&& w, std::shared_future<Results>... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return dispatch([w](std::shared_future<Results>... f) mutable {
                return w(f.get()...);
            }, std::shared_future<Results>(f)...);   
        }


        template <class Work, class... Results>
        auto when_all(connection& c, Work&& w, std::future<Results>&&... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return when_all(c, std::forward<Work>(w), f.share()...);
        }

        template <class Work, class... Results>
        auto when_all(connection& c, Work&& w, std::shared_future<Results>... f) noexcept -> std::future<decltype(w(f.get()...))>
        {
            return dispatch(c, [w](std::shared_future<Results>... f) mutable {
                return w(f.get()...);
            }, std::shared_future<Results>(f)...);   
        }

        bool process_all() noexcept;
        bool process_one() noexcept;

    };
}


#endif