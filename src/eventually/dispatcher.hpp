
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/task_queue.hpp>

namespace eventually {

    class petition;

    class dispatcher
    {
    private:
        task_queue _queue;
    public:
        petition dispatch(const task::handler& handler);
        void process();
    };

}


#endif