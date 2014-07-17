
#include <eventually/dispatcher.hpp>

namespace eventually {

    bool dispatcher::process_all() noexcept
    {
        bool result_ = false;
        while(process_one())
        {
            result_ = true;
        }

        return result_;
    }

    bool dispatcher::process_one() noexcept
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
