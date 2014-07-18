
#ifndef _eventually_connection_hpp_
#define _eventually_connection_hpp_

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
        std::atomic_bool interrupt_flag;
        std::mutex _mutex;

        connection_data():
        interrupt_flag(false)
        {
        }
    };

    /**
     * The exception thrown when a connection is working
     * and is interrupted.
     */
    class connection_interrupted : public std::exception
    {
        virtual const char* what() const throw();
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
        void interrupt() noexcept;
        void interruption_point();

        template <typename Work>
        auto get_work_done(Work&& w) -> decltype(w())
        {
            std::lock_guard<std::mutex> lock_(_data->_mutex);
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