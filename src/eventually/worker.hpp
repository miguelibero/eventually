
#ifndef _eventually_worker_hpp_
#define _eventually_worker_hpp_

#include <memory>
#include <atomic>
#include <mutex>
#include <vector>
#include <eventually/is_callable.hpp>
#include <eventually/connection.hpp>

namespace eventually {

    template<typename FinalResult>
    struct when_any_worker_data
    {
        size_t size;
        bool worked;
        std::promise<FinalResult> promise;
        std::mutex mutex;
        connection conn;

        when_any_worker_data(size_t size, connection& c):
        size(size), worked(false), conn(c)
        {
        }

        when_any_worker_data(size_t size):
        size(size), worked(false)
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

        void catch_exception()
        {
            std::lock_guard<std::mutex> lock_(_data->mutex);
            if(!_data->worked)
            {
                --_data->size;
                if(_data->size == 0)
                {
                    _data->promise.set_exception(std::current_exception());
                }
            }
        }

    public:
        when_any_worker(size_t size):
        _data(std::make_shared<when_any_worker_data<FinalResult>>(size))
        {
        }

        when_any_worker(size_t size, connection& c):
        _data(std::make_shared<when_any_worker_data<FinalResult>>(size, c))
        {
        }

        template <typename Work, typename Result>
        void work(Work& w, std::future<Result>& f)
        {
            try
            {
                _data->conn.interruption_point();
                _data->mutex.lock();
                if(!_data->worked)
                {
                    _data->promise.set_value(w(f.get()));
                    _data->worked = true;
                    _data->mutex.unlock();
                }
                else
                {
                    _data->mutex.unlock();
                    f.wait();
                }
            }
            catch(...)
            {
                catch_exception();
            }
        }

        template <typename Work>
        void work(Work& w, std::future<void>& f)
        {
            try
            {
                _data->conn.interruption_point();
                _data->mutex.lock();
                if(!_data->worked)
                {
                    f.wait();
                    _data->promise.set_value(w());
                    _data->worked = true;
                    _data->mutex.unlock();
                }
                else
                {
                    _data->mutex.unlock();
                    f.wait();
                }
            }
            catch(...)
            {
                catch_exception();
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
        connection conn;        

        when_every_worker_data(size_t size, connection& c):
        size(size), conn(c)
        {
            results.reserve(size);
        }

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

        template <typename Work,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        void step(Work& w)
        {
            w(_data->results);
            if(_data->size == _data->results.size())
            {
                _data->promise.set_value(_data->results);
            }
        }

    public:
        when_every_worker(size_t size):
        _data(std::make_shared<when_every_worker_data<Result>>(size))
        {
        }

        when_every_worker(size_t size, connection& c):
        _data(std::make_shared<when_every_worker_data<Result>>(size, c))
        {
        }

        template <typename Work,
            typename std::enable_if<is_callable<Work(Container&)>::value, int>::type = 0>
        void work(Work& w, std::future<Result>& f)
        {
            try
            {
                _data->conn.interruption_point();
                Result r(f.get());
                std::lock_guard<std::mutex> lock_(_data->mutex);
                _data->results.push_back(r);
                step(w);
            }
            catch(...)
            {
                std::lock_guard<std::mutex> lock_(_data->mutex);
                _data->size--;
                step(w);
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
        static auto work(Work& w, std::future<Results>&... fs) -> decltype(w(fs.get()...))
        {
            return w(fs.get()...);
        }

        template <typename Work,
            typename std::enable_if<is_callable<Work()>::value, int>::type = 0>
        static auto work(Work& w, std::future<void>& f) -> decltype(w())
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
        static auto work(Work& w, std::future<Result>& f) -> Result
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
        static auto work(Work& w, std::future<void>& f) -> void
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
