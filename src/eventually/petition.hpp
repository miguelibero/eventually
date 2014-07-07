
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
        inline void make_equal_to(const petition& other);
    public:
        inline petition();
        inline petition(const petition& other);
        inline bool close();
        inline bool active() const;
        inline petition& operator=(const petition& other);
        inline const petition operator>>(petition other) const;
    };

    class petition_data
    {
    private:
        std::atomic<bool> _active;
    public:
        inline petition_data();
        inline bool close();
        inline bool active() const;
    };

}

#include <eventually/petition.impl.hpp>

#endif