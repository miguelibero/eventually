#ifndef _eventually_file_data_loader_hpp_
#define _eventually_file_data_loader_hpp_

#include <eventually/data_loader.hpp>

namespace eventually {

    class dispatcher;
    class connection;

    class file_data_loader
    {
    private:
        dispatcher* _dispatcher;
        bool _delete_dispatcher;
        size_t _block_size;

    public:
        static const size_t nblock;

        file_data_loader(size_t block);
        file_data_loader(dispatcher* d=nullptr, size_t block=nblock);
        file_data_loader(dispatcher& d, size_t block=nblock);

        ~file_data_loader();

        dispatcher& get_dispatcher();
        std::future<data> load(connection& c, const std::string& name);
        std::future<data> load(const std::string& name);
    };
}

#endif
