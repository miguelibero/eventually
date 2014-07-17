
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task.hpp>
#include <eventually/connection.hpp>
#include <deque>
#include <memory>
#include <mutex>

namespace eventually {

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
            std::unique_ptr<task<decltype(w(args...))>> task_(
                new task<decltype(w(args...))>(w, args...));
            auto future_ = task_->get_future();
            std::lock_guard<std::mutex> lock_(_mutex);
            _tasks.push_back(std::move(task_));
            return future_;
        }

        template<class Work, class... Args>
        auto dispatch(connection& c, Work&& w, Args&&... args) noexcept ->  std::future<decltype(w(args...))>
        {
            std::unique_ptr<task<decltype(w(args...))>> task_(
                new task<decltype(w(args...))>(c, w, args...));
            auto future_ = task_->get_future();
            std::lock_guard<std::mutex> lock_(_mutex);
            _tasks.push_back(std::move(task_));
            return future_;
        }

        template <class Result, class Work>
        auto then(std::future<Result>&& f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return then(f.share(), w);
        }

        template <class Result, class Work>
        auto then(std::shared_future<Result> f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch([](std::shared_future<Result> f, Work w){
                return get_work_done(f, w);
            }, f, w);
        }

        template <class Result, class Work>
        auto then(connection& c, std::future<Result>&& f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return then(c, f.share(), w);
        }

        template <class Result, class Work>
        auto then(connection& c, std::shared_future<Result> f, Work&& w) noexcept -> std::future<decltype(w(f.get()))>
        {
            return dispatch(c, [](std::shared_future<Result> f, Work w){
                return get_work_done(f, w);
            }, f, w);
        }

        bool process_all() noexcept;
        bool process_one() noexcept;

    };
}


#endif