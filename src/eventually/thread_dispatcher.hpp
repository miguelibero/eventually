
#ifndef _eventually_thread_dispatcher_hpp_
#define _eventually_thread_dispatcher_hpp_

#include <eventually/dispatcher.hpp>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>

namespace eventually {

    /**
     * Calls dispatcher process on a set of threads
     */
    class thread_dispatcher : public dispatcher
    {

    public:
        typedef std::chrono::duration<double> duration;
    private:
        duration _wait;
        std::vector<std::thread> _threads;
        std::atomic_bool _done;

        void worker_thread(size_t i);
    public:
        thread_dispatcher(const duration& wait=duration::zero(), size_t thread_count=std::thread::hardware_concurrency());
        ~thread_dispatcher();
    };

}

#endif