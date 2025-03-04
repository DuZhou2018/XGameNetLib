#include "../header/NiceIDMg.h"
#include "../header/CGlobal.hpp"
#include "../header/CountersMg.hpp"
#include "../header/InPublicPreDefine.hpp"

namespace peony
{
    namespace net
    {
		extern unsigned g_RunMsgEvent;
        CNiceIDMG::CNiceIDMG(void):m_mutex(*PNGB::m_globMutex)
        {
            m_pCtMsg = INiceNetCounter::CreateCounter((string(PN_CU_PATH)+"JarMsg").c_str(),"JarFreeCount","当前消息队列消息数量!",true);
        }
        CNiceIDMG::~CNiceIDMG(void)
        {
            INiceNetCounter::DeleteCounter(m_pCtMsg);
        }
        void CNiceIDMG::UnInit()
        {
            CAutoLogDeadlock AutoLgDL(__FUNCTION__);
            m_pCtMsg->UnRegister();
        }
        void CNiceIDMG::PutMsgFun(on_netmsg_hand funhand,bool isinserthead/*=false*/)
        {
            Boost_Scoped_Lock MsgFun_lock(m_mutex);
			if( isinserthead )
			{
				m_high_level_FunQueue.push(funhand);
			}
			else
	            m_FunQueue.push(funhand);
        }

        unsigned CNiceIDMG::Run(unsigned uMaxCount,unsigned curtime )
        {
            //lock range min
			m_msg_q_size = 0;
            unsigned iDoCount = 0;
			//先处理连接的队列
			iDoCount += Run_high_level_FunQueue( 50 );
            do
            {
                Boost_Scoped_Lock MsgFun_lock(m_mutex);
				while(iDoCount<uMaxCount)
                {
                    if( m_FunQueue.empty() )
                        break;

                    iDoCount++;
					g_RunMsgEvent++;
                    on_netmsg_hand &MsgFun = m_FunQueue.front();

                    MsgFun_lock.unlock();
                    MsgFun();
                    MsgFun_lock.lock();
                    m_FunQueue.pop();
                }
				m_msg_q_size  = m_FunQueue.size();
				m_msg_q_size += m_high_level_FunQueue.size();

            }while(0);
			//iDoCount += Run_high_level_FunQueue( 10 );

			if(m_msg_q_size>10){
				NETLOG_ERROR("[当前未处理的消息] m_msg_q_size="<<m_msg_q_size<<FUN_FILE_LINE );
			}
			return iDoCount;
        }

		unsigned CNiceIDMG::Run_high_level_FunQueue( unsigned uCount )
		{
			//lock range min
			unsigned iDoCount = 0;
			do
			{
				Boost_Scoped_Lock MsgFun_lock(m_mutex);
				while(iDoCount<uCount)
				{
					if( m_high_level_FunQueue.empty() )
						break;

					iDoCount++;
					on_netmsg_hand &MsgFun = m_high_level_FunQueue.front();
					MsgFun_lock.unlock();
					MsgFun();
					MsgFun_lock.lock();
					m_high_level_FunQueue.pop();
				}
			}while(0);
			return iDoCount;
		}
		void CNiceIDMG::UpdateCounter()
		{
			if( m_pCtMsg )
			    m_pCtMsg->UpdateReplace( m_msg_q_size );
		}
	}
}