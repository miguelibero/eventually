
#ifndef _eventually_thread_dispatcher_hpp_
#define _eventually_thread_dispatcher_hpp_

#include <eventually/dispatcher.hpp>
#include <chrono>

namespace eventually {

    class thread_dispatcher : public dispatcher
    {
    public:
    	typedef std::chrono::duration<double> duration;
    	
        thread_dispatcher(size_t threads, const duration& wait=duration::zero());
    };

}

#endif