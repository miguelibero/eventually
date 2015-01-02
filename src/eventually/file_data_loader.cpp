
#include <eventually/file_data_loader.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/connection.hpp>
#include <eventually/thread_dispatcher.hpp>
#include <functional>
#include <mutex>
#include <cstdio>

namespace eventually {

    const size_t file_data_loader::nblock = -1;

    file_data_loader::file_data_loader(size_t block_size):
    _dispatcher(new thread_dispatcher()),
    _delete_dispatcher(true), _block_size(block_size)
    {
    }

    file_data_loader::file_data_loader(dispatcher* d, size_t block_size):
    _dispatcher(d ? d : new thread_dispatcher()),
    _delete_dispatcher(true), _block_size(block_size)
    {
    }

    file_data_loader::file_data_loader(dispatcher& d, size_t block_size):
    _dispatcher(&d),
    _delete_dispatcher(false), _block_size(block_size)
    {
    }

    file_data_loader::~file_data_loader()
    {
        if(_delete_dispatcher)
        {
            delete _dispatcher;
        }
    }

    FILE* file_data_loader_start(const std::string& name)
    {
		FILE *fh = nullptr;
#ifdef _MSC_VER
		if(fopen_s(&f, name.c_str(), "rb") != 0)
		{
			fh = nullptr;
		}
#else
        fh = fopen(name.c_str(), "rb");
#endif
		if (fh == nullptr)
        { 
            throw data_exception(std::string("Could not open file '")+name+"'.");
        }
        return fh;
    }

    std::mutex work_mutex;

    bool file_data_loader_work(data& d, FILE* fh, size_t block_size)
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        uint8_t b;
        while(block_size!=0)
        {
            if (fread(&b, 1, 1, fh) == 0) 
            {
                return true;
            }
            d.push_back(b);
            if(block_size != file_data_loader::nblock)
            {
                block_size--;
            }
        }
        return false;
    }

    data file_data_loader_end(data&& d, FILE* fh)
    {
        fclose(fh);
        return std::move(d);
    }

    void file_data_loader_throw(const data_exception& err, FILE* fh)
    {
        fclose(fh);
        throw err;
    }

    dispatcher& file_data_loader::get_dispatcher()
    {
    	return *_dispatcher;
    }

    std::future<data> file_data_loader::load(const std::string& name)
    {
        connection conn;
        return load(conn, name);
    }

    std::future<data> file_data_loader::load(connection& c, const std::string& name)
    {
        if(!_dispatcher)
        {
            throw new data_exception("No dispatcher found.");
        }
        FILE* fh = file_data_loader_start(name);
        return _dispatcher->when_throw<data_exception>(
            std::bind(&file_data_loader_throw, std::placeholders::_1, fh),
            _dispatcher->dispatch_retry(c, 
                std::bind(&file_data_loader_work, std::placeholders::_1, fh, _block_size),
                std::bind(&file_data_loader_end, std::placeholders::_1, fh),
                data()));
    }

}
