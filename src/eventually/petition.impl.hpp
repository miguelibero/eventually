
#ifndef _eventually_petition_impl_hpp_
#define _eventually_petition_impl_hpp_

#include <eventually/petition.hpp>

namespace eventually {

    petition::petition():
    _data(new std::shared_ptr<petition_data>(std::make_shared<petition_data>()))
    {
    }

    petition::petition(const petition& other):
    _data(other._data)
    {
    }

    bool petition::close()
    {
        std::lock_guard<decltype(_data_mutex)> lock(_data_mutex);
        return (*_data)->close();
    }

    bool petition::active() const
    {
        std::lock_guard<decltype(_data_mutex)> lock(_data_mutex);
        return (*_data)->active();
    }

    void petition::make_equal_to(const petition& other) const
    {
        std::lock_guard<decltype(_data_mutex)> lock(_data_mutex);
        if(other._data != _data || *other._data != *_data)
        {
            std::lock_guard<decltype(other._data_mutex)> lock(other._data_mutex);
            *_data = *other._data;
            other._data = _data;
        }
    }

    petition& petition::operator=(const petition& other)
    {
        make_equal_to(other);
        return *this;
    }

    petition& petition::operator>>(const petition& other)
    {
        other.make_equal_to(*this);
        return *this;
    }

    petition_data::petition_data():
    _active(true)
    {
    }

    bool petition_data::close()
    {
        return _active.exchange(false);
    }

    bool petition_data::active() const
    {
        return _active.load();
    }

}

#endif