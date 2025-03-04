#ifndef __0MP_NET_SHEDULER_HEADER_FILE__
#define __0MP_NET_SHEDULER_HEADER_FILE__
#include "./Scheduler_impl.hpp"

namespace peony {
	namespace net {

		class   Scheduler_impl;
		class Scheduler
		{			
		private:			
            Scheduler_impl  *impl_ptr;
        public:
            Scheduler(void);
            ~Scheduler(void);

		public:
			AsioStrand     &get_strand() { return impl_ptr->get_strand(); }
			io_context     &get_ioservice() { return impl_ptr->get_ioservice(); }

			Scheduler_impl *get_impl(){ return impl_ptr;};
			void            startup(unsigned uthreadnum);
			void            shutdown();
			void            post(post_handler_type handler);
		};

	}	// end of namespace net
}	// end of namespace peony

#endif//#define __0MP_NET_SHEDULER_HEADER_FILE__