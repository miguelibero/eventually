
#ifndef _eventually_petition_hpp_
#define _eventually_petition_hpp_

#include <memory>
#include <atomic>
#include <mutex>

namespace eventually {

    class petition_data;

    class petition
    {
    private:
        mutable std::mutex _data_mutex;
        mutable std::shared_ptr<std::shared_ptr<petition_data>> _data;
        void make_equal_to(const petition& other);
    public:
        petition();
        petition(const petition& other);
        bool close();
        bool active() const;
        petition& operator=(const petition& other);
        const petition operator>>(petition other) const;
    };

    class petition_data
    {
    private:
        std::atomic<bool> _active;
    public:
        petition_data();
        bool close();
        bool active() const;
    };

}

#endif