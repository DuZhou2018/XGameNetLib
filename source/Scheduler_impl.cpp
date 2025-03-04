#include "../header/NiceLog.h"
#include "../header/CGlobal.hpp"
#include "../header/Scheduler_impl.hpp"
#include "../header/InPublicPreDefine.hpp"

#ifdef WIN32
#pragma warning(disable:4503)
#endif

namespace peony {
	namespace net {

		Scheduler_impl::Scheduler_impl(void):m_strand(m_ioservice),m_is_running(false)
		{
			m_pTimer = 0;
		}

		Scheduler_impl::~Scheduler_impl(void)
		{
		}
		void Scheduler_impl::startup(unsigned uthreadnum)
		{
			if(m_is_running)
				return;

			if(!m_thread_pool.empty())
			{// may be in the progress of shutdown
				return;
			}
			
			add_timer();
			m_is_running=true;
			for(unsigned i=0;i<uthreadnum;i++ )
			{
				boost::thread * new_thread(new boost::thread( boost::bind(&Scheduler_impl::thread_func,this) ));
				m_thread_pool.push_back( ZThreadItem(new_thread) );
				NETLOG_SYSINFO("[创建线程.网络收发] threadid="<<CInPubFun::GetThreadId(new_thread));
			}
		}

		unsigned Scheduler_impl::add_timer()
		{
			if( 0==m_pTimer ){
				m_pTimer = new boost::asio::deadline_timer(m_ioservice);
			}
			ZBoostErrCode ec;
			m_pTimer->expires_from_now( boost::posix_time::milliseconds(1),ec );
			if(ec){
				return 0;
			}
			
			m_pTimer->async_wait(m_strand.wrap(boost::bind(&Scheduler_impl::timer_handler,this,boost::asio::placeholders::error, 1)));
			return 0;
		}
		void Scheduler_impl::timer_handler(const ZBoostErrCode& error,unsigned index)
		{
			add_timer();
		}

		bool Scheduler_impl::shutdown()
		{
			if(!m_is_running)
			{
				m_ioservice.stop();
				m_thread_pool.clear();
				return false;
			}
			else
			{
                NETLOG_NORMAL("waiting for all thread... ");
				m_is_running=false;
				m_ioservice.stop();

				{
					boost::this_thread::disable_interruption di;
					for( ThreadPool::iterator i = m_thread_pool.begin();i != m_thread_pool.end(); ++i)
					{//wait thread over
						ZThreadItem *pItem = &(*i);
						pItem->pThread->join();
						NETLOG_SYSINFO("[删除线程.网络收发] threadid="<<pItem->uThreadID );
						delete (pItem->pThread);
					}
                    m_thread_pool.clear();
				}
				return true;
			}
		}

		void Scheduler_impl::thread_func()
		{
			do{
				try{
					if( 0 == m_ioservice.run() )
						boost::this_thread::sleep(boost::posix_time::milliseconds(1));
				}catch(...)
				{
                    NETLOG_FATAL("[Scheduler_impl::thread_func] exception!! "<<FUN_FILE_LINE);
					PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				}
			}while(m_is_running);
		}
        void Scheduler_impl::post(post_handler_type handler)
        {
           m_strand.post(handler);
        }
	}	// end of namespace net
}	// end of namespace peony