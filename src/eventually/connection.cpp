
#include <eventually/connection.hpp>

namespace eventually {

    const char* connection_interrupted::what() const THROW
    {
        return "connection interrupted";    
    }

    connection_data::connection_data()
    {
        _interrupt_flag.store(false);
    }


    void connection_data::interrupt() NOEXCEPT
    {
        _interrupt_flag.store(true);
    }

    void connection_data::interruption_point()
    {
        if(_interrupt_flag.load())
        {
            throw connection_interrupted();
        }
    }

    connection::connection():
    _data(std::make_shared<connection_data>())
    {
    }

    connection::~connection()
    {
        std::lock_guard<std::mutex> lock(get_mutex());
    }

    void connection::interrupt() NOEXCEPT
    {
        _data->interrupt();
    }

    void connection::interruption_point()
    {
        _data->interruption_point();
    }

    std::mutex& connection::get_mutex() NOEXCEPT
    {
        return _data->_mutex;
    }

    scoped_connection::~scoped_connection()
    {
        interrupt();
    }

}
