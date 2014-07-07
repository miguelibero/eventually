
#include <eventually/task_queue.hpp>

namespace eventually {

    task_queue::task_queue()
    {
    }

    petition task_queue::push_front(task data)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push_front(std::move(data));
        return _queue.front().get_petition();
    }

    petition task_queue::push_back(task data)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push_back(std::move(data));
        return _queue.back().get_petition();
    }

    bool task_queue::empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    bool task_queue::try_pop_front(task& res)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty())
        {
            return false;
        }
        
        res=std::move(_queue.front());
        _queue.pop_front();
        return true;
    }

    bool task_queue::try_pop_back(task& res)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty())
        {
            return false;
        }
        
        res=std::move(_queue.back());
        _queue.pop_back();
        return true;
    }
};