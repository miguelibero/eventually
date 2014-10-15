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

        static data_ptr load_dispatched(connection& c, const std::string& name);

    public:

        file_data_loader(dispatcher* d=nullptr);
        file_data_loader(dispatcher& d);

        ~file_data_loader();

        dispatcher& get_dispatcher();
        std::future<data_ptr> load(connection& c, const std::string& name);
        std::future<data_ptr> load(const std::string& name);
	};
}

#endif