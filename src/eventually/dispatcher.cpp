
#include <eventually/dispatcher.hpp>

namespace eventually {

    dispatcher::~dispatcher()
    {
    }

    bool dispatcher::process_all() NOEXCEPT
    {
        bool result_ = false;
        while(process_one())
        {
            result_ = true;
        }

        return result_;
    }

    bool dispatcher::process_one() NOEXCEPT
    {
        std::lock_guard<std::mutex> lock_(_mutex);
        if(!_tasks.empty())
        {
            basic_task_ptr& task_ = _tasks.front();
            if((*task_)())
            {
                _tasks.pop_front();
            }
            return true;
        }
        return false;
    }

}
