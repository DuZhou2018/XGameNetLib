#ifndef __0MP_NET_SHEDULER_IMPL_HEADER_FILE__
#define __0MP_NET_SHEDULER_IMPL_HEADER_FILE__
#include "./ANetVersion.hpp"
#include "./InPublicPreDefine.hpp"

namespace peony {
	namespace net {
		typedef boost::function<void(void)> post_handler_type;

		class Scheduler_impl
		{
		private:
			io_context	                    m_ioservice;		
			AsioStrand						m_strand;

			bool							m_is_running;
			ThreadPool						m_thread_pool;
			boost::asio::deadline_timer *	m_pTimer;

        public:
            // ONLY for internal class use
            inline AsioStrand &get_strand(){return m_strand;}
            inline io_context &get_ioservice(){return m_ioservice;}
			void   post(post_handler_type handler);

		private:
            friend class Scheduler;
			Scheduler_impl(void);
            ~Scheduler_impl(void);

			void			startup( unsigned uthreadnum);
			bool			shutdown();
			void			thread_func();
			unsigned		add_timer();
			void			timer_handler(const ZBoostErrCode& error,unsigned index );
		};

	}	// end of namespace net
}	// end of namespace peony

#endif//#define __0MP_NET_SHEDULER_IMPL_HEADER_FILE__