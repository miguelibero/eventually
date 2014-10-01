
#ifndef _eventually_connection_hpp_
#define _eventually_connection_hpp_

#include <eventually/define.hpp>
#include <memory>
#include <exception>
#include <mutex>
#include <atomic>

namespace eventually {

    /**
     * The shared data between connection handlers
     */
    struct connection_data
    {
        std::atomic_bool _interrupt_flag;
        std::mutex _work_mutex;

        connection_data();
        void interrupt() NOEXCEPT;
        void interruption_point();
    };

    /**
     * The exception thrown when a connection is working
     * and is interrupted.
     */
    class connection_interrupted : public std::exception
    {
        virtual const char* what() const THROW;
    };

    /**
     * A handler class for interrupting dispatched works.
     */
    class connection
    {
    private:
        std::shared_ptr<connection_data> _data;
    public:
        connection();
        virtual ~connection();
        void interrupt() NOEXCEPT;
        void interruption_point();

        template <typename Work>
        auto work(Work&& w) -> decltype(w())
        {
            std::lock_guard<std::mutex> lock_(_data->_work_mutex);
            interruption_point();
            return w();
        }
    };

    /**
     * A connection that will automatically interrupt when destroyed.
     */
    class scoped_connection : public connection
    {
    public:
        virtual ~scoped_connection();
    };

}

#endif
