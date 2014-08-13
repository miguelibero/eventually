
#ifndef _eventually_worker_hpp_
#define _eventually_worker_hpp_

#include <memory>
#include <atomic>
#include <tuple>
#include <eventually/apply.hpp>

namespace eventually {

    template<class FinalResult>
    struct shared_worker_data
    {
        std::promise<FinalResult> promise;
        std::atomic_flag flag;

        shared_worker_data():
        flag(ATOMIC_FLAG_INIT)
        {
        }
    };

    /**
     * Used to share a promise between all the futures
     * when calling dispatcher::when_any
     */
    template<class FinalResult>
    class shared_worker
    {
    private:
        std::shared_ptr<shared_worker_data<FinalResult>> _data;
    public:
        shared_worker():
        _data(std::make_shared<shared_worker_data<FinalResult>>())
        {
        }

        template <typename Work, typename... Results>
        bool get_work_done(Work& w, std::shared_future<Results>&... fs)
        {
            std::tuple<Results...> rs(fs.get()...);
            if(!_data->flag.test_and_set())
            {
                _data->promise.set_value(apply(w, rs));
                return true;
            }
            else
            {
                return false;
            }
        }

        template <typename Work>
        bool get_work_done(Work& w, std::shared_future<void>& f)
        {
            f.get();
            if(_data->flag.test_and_set())
            {
                _data->promise.set_value(w());
                return true;
            }
            else
            {
                return false;
            }
        }

        std::future<FinalResult> get_future()
        {
            return _data->promise.get_future();
        }
    };

    /**
     * Used to get the future and pass it to a work function object
     */
    class worker
    {
    public:
        template <typename Work, typename... Results>
        static auto get_work_done(Work& w, std::shared_future<Results>&... fs) -> decltype(w(fs.get()...))
        {
            return w(fs.get()...);
        }

        template <typename Work>
        static auto get_work_done(std::shared_future<void>& f, Work& w) -> decltype(w())
        {
            f.wait();
            return w();
        }
    };

}


#endif
