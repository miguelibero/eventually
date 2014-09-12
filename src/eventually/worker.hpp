
#ifndef _eventually_worker_hpp_
#define _eventually_worker_hpp_

#include <memory>
#include <atomic>
#include <mutex>
#include <vector>
#include <eventually/is_callable.hpp>

namespace eventually {

    template<typename FinalResult>
    struct when_any_worker_data
    {
        std::promise<FinalResult> promise;
        std::atomic_flag flag;

        when_any_worker_data():
        flag(ATOMIC_FLAG_INIT)
        {
        }
    };

    /**
     * Used to share a promise between all the futures
     * when calling dispatcher::when_any
     */
    template<typename FinalResult>
    class when_any_worker
    {
    private:
        std::shared_ptr<when_any_worker_data<FinalResult>> _data;
    public:
        when_any_worker():
        _data(std::make_shared<when_any_worker_data<FinalResult>>())
        {
        }

        template <typename Work, typename Result>
        bool work(Work& w, std::shared_future<Result>& f)
        {
            if(!_data->flag.test_and_set())
            {
                _data->promise.set_value(w(f.get()));
                return true;
            }
            else
            {
                f.get();
                return false;
            }
        }

        template <typename Work>
        bool work(Work& w, std::shared_future<void>& f)
        {
            f.wait();
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

    template<typename Result,
        typename Container = std::vector<Result>>
    struct when_every_worker_data
    {
        size_t size;
        Container results;
        std::mutex mutex;
        std::promise<Container> promise;

        when_every_worker_data(size_t size):
        size(size)
        {
            results.reserve(size);
        }
    };

    /**
     * Used to share a promise between all the futures
     * when calling dispatcher::when_every
     */
    template<typename Result,
        typename Container = std::vector<Result>>
    class when_every_worker
    {
    private:
        std::shared_ptr<when_every_worker_data<Result>> _data;
    public:
        when_every_worker(size_t size):
        _data(std::make_shared<when_every_worker_data<Result>>(size))
        {
        }

        template <typename Work,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        void work(Work& w, std::shared_future<Result>& f)
        {
            std::lock_guard<std::mutex> lock_(_data->mutex);
            _data->results.push_back(f.get());
            w(_data->results);
            if(_data->size == _data->results.size())
            {
                _data->promise.set_value(_data->results);
            }
        }

        std::future<Container> get_future()
        {
            return _data->promise.get_future();
        }
    };

    /**
     * Used to get the future and pass it to a work function object
     */
    class when_worker
    {
    private:
        when_worker();
    public:
        template <typename Work, typename... Results,
            typename std::enable_if<is_callable<Work(Results...)>::value, int>::type = 0>
        static auto work(Work& w, std::shared_future<Results>&... fs) -> decltype(w(fs.get()...))
        {
            return w(fs.get()...);
        }

        template <typename Work,
            typename std::enable_if<is_callable<Work()>::value, int>::type = 0>
        static auto work(Work& w, std::shared_future<void>& f) -> decltype(w())
        {
            f.wait();
            return w();
        }
    };

    /**
     * Used to catch an exception when getting a future
     */
    class when_throw_worker
    {
    private:
        when_throw_worker();
    public:        
        template <typename Work, typename Result, typename Exception = std::exception,
            typename std::enable_if<is_callable_with_result<Work(const Exception&), Result>::value, int>::type = 0>
        static auto work(Work& w, std::shared_future<Result>& f) -> Result
        {
            try
            {
                return f.get();
            }
            catch(const Exception& e)
            {
                return w(e);
            }
        }

        template <typename Work, typename Exception = std::exception,
        typename std::enable_if<is_callable<Work(const Exception&)>::value, int>::type = 0>
        static auto work(Work& w, std::shared_future<void>& f) -> void
        {
            try
            {
                f.wait();
            }
            catch(const Exception& e)
            {
                w(e);
            }
        }
    };

}


#endif
