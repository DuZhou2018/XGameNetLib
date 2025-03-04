#include "../header/Scheduler.hpp"


#include "../header/CGlobal.hpp"
#include "../header/NiceLog.h"
#include "../header/NiceLog.h"

namespace peony {
	namespace net {
		Scheduler::Scheduler(void):impl_ptr(new Scheduler_impl())
		{
		}

		Scheduler::~Scheduler(void)
		{
            delete impl_ptr;
            impl_ptr = 0;
		}
		//	call to pimpl.obj.
		void Scheduler::startup(unsigned uthreadnum)
		{
			impl_ptr->startup( uthreadnum );
		}

		void Scheduler::shutdown()
		{
           impl_ptr->shutdown();
		}

        void Scheduler::post(post_handler_type handler)
        {
            impl_ptr->post(handler);
        }

	}	// end of namespace net

}	// end of namespace peony
