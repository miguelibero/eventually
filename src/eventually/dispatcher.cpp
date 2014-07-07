
#ifndef _eventually_dispatcher_impl_hpp_
#define _eventually_dispatcher_impl_hpp_

#include <eventually/dispatcher.hpp>

namespace eventually {

    petition dispatcher::dispatch(const task::handler& handler)
    {
        return _queue.push_back(handler);
    }

    void dispatcher::process()
    {
        task t;
        while(_queue.try_pop_front(t))
        {
            t();   
        }
    }

}

#endif