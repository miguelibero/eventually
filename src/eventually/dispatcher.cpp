
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
        basic_task_ptr task_;
        {
            std::lock_guard<std::mutex> lock_(_mutex);
            if(_tasks.empty())
            {
                return false;
            }
            task_ = std::move(_tasks.front());
            _tasks.pop_front();
        }
        (*task_)();
        return true;
    }

}
