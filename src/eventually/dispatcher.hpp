
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task.hpp>
#include <eventually/connection.hpp>
#include <eventually/worker.hpp>
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
        auto when(Work&& w, std::future<Result>&& f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(std::forward<Work>(w), f.share());
        }

        template <class Work, class Result>
        auto when(Work&& w, std::shared_future<Result> f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch([w, f]() mutable {
                return worker::get_work_done(w, f);
            });
        }

        template <class Work, class Result>
        auto when(connection& c, Work&& w, std::future<Result>&& f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when(c, std::forward<Work>(w), f.share());
        }

        template <class Work, class Result>
        auto when(connection& c, Work&& w, std::shared_future<Result> f) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [w, f]() mutable {
                return worker::get_work_done(w, f);
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
                return worker::get_work_done(w, f...);
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
                return worker::get_work_done(w, f...);
            }, std::shared_future<Results>(f)...);
        }

        template <class Work, class Result, class... Results>
        auto when_any(Work&& w, std::future<Result>&& f, std::future<Results>&&... fs) noexcept -> std::future<decltype(w(f.get()))>
        {
            return when_any(std::forward<Work>(w), f.share(), fs.share()...);
        }

        template <class Work, class Result, class... Results>
        auto when_any(Work&& w, std::shared_future<Result> f, std::shared_future<Results>... fs) noexcept -> std::future<decltype(w(f.get()))>
        {
            shared_worker<decltype(w(f.get()))> p;
            when_any(std::forward<Work>(w), p, f, fs...);
            return p.get_future();
        }

        template <class Work, class FinalResult, class Result, class... Results>
        void when_any(Work&& w, shared_worker<FinalResult> p, std::shared_future<Result> f, std::shared_future<Results>... fs) noexcept
        {
            when_any(std::forward<Work>(w), p, fs...);
            when_any(std::forward<Work>(w), p, f);
        }

        template <class Work, class FinalResult, class Result>
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