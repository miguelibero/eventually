
#include <eventually/thread_dispatcher.hpp>
#include <iostream>

namespace eventually {

    thread_dispatcher::thread_dispatcher(const duration& wait, size_t thread_count):
    _wait(wait), _done(false)
    {
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
            _done = true;
            throw;
        }
    }

    thread_dispatcher::~thread_dispatcher()
    {
        {
            std::unique_lock<std::mutex> lock_(_wait_mutex);
            _done = true;
            _new_task.notify_all();
        }
        for(auto& thread_ : _threads)
        {
            thread_.join();
        }
    }

    void thread_dispatcher::worker_thread(size_t i)
    {
        while(!_done)
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
