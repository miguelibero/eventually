
#include <eventually/thread_dispatcher.hpp>

namespace eventually {

    thread_dispatcher::thread_dispatcher(size_t thread_count):
    _wait(duration::zero())
    {
        init(thread_count);
    }

    thread_dispatcher::thread_dispatcher(const duration& wait, size_t thread_count):
    _wait(wait)
    {
        init(thread_count);
    }

    void thread_dispatcher::init(size_t thread_count)
    {
        _done.store(false);
        try
        {
            for(size_t i=0; i<thread_count; ++i)
            {
                _threads.push_back(
                    std::thread(&thread_dispatcher::worker_thread, this, i));
            }
        }
        catch(...)
        {
            _done.store(true);
            throw;
        }
    }

    thread_dispatcher::~thread_dispatcher()
    {
        {
            std::unique_lock<std::mutex> lock_(_wait_mutex);
            _done.store(true);
            _new_task.notify_all();
        }
        for(auto& thread_ : _threads)
        {
            thread_.join();
        }
    }

    void thread_dispatcher::worker_thread(size_t i)
    {
        while(!_done.load())
        {
            while(process_one())
            {
                if(_wait == duration::zero())
                {
                    std::this_thread::yield();
                }
                else
                {
                    std::this_thread::sleep_for(_wait);
                }
            }
            std::unique_lock<std::mutex> lock_(_wait_mutex);
            if(!_done)
            {
                _new_task.wait(lock_);
            }
        }
    }

}
