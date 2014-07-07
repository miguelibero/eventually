
#include <eventually/task.hpp>

namespace eventually {

	task::task(const handler& handler):
	_handler(handler)
	{
	}

	petition& task::get_petition()
	{
		return _petition;
	}
	
	void task::operator()()
	{
		if(_petition.active() && _handler != nullptr)
		{
			_handler();
		}
	}

}