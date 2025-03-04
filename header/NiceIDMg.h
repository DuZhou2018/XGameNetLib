#ifndef NICE_NET_ID_SCS_MG
#define NICE_NET_ID_SCS_MG

#include "../header/TCPClientSession.hpp"
#include "../header/TCPServerSession.hpp"

#include "../header/QueueSafe.h"
#include "../include/INiceLog.h"
#include "../include/INiceNetCounter.h"

namespace peony
{
    namespace net
    {
        typedef boost::function<void()>on_netmsg_hand;
        typedef CQueueSafe<on_netmsg_hand> TMsgFunQueue;
        class CNiceIDMG
        {
        public:
            CNiceIDMG(void);
            ~CNiceIDMG(void);

            void UnInit();
			void UpdateCounter();
        public:
			/********************************************************************
				created:	2009/11/16	16:11:2009   14:55
				author:		zhanghongtao	
				purpose:	run函数必须保证在一个线程中调用，否则出出现严重错误
				如果忙返回 true
			*********************************************************************/
            unsigned        Run(unsigned uMaxCount,unsigned curtime);

			/********************************************************************
			created:	10:11:2010   9:58
			author:		zhanghongtao	
			purpose:	isinserthead,新添加了参数，是否把这个事件插入到事件队列的头，
						也就是是否有先处理这个消息。
						现在使用的事件有：TCPServer的网络连接事件，因为如果不优先处理，
						会使客户端的连接变的非常的慢，因为大部分时候网络没有监听状态。
			*********************************************************************/
            void			PutMsgFun(on_netmsg_hand funhand,bool isinserthead=false);
		private:
			unsigned	Run_high_level_FunQueue(unsigned uCount);

        private:
            TMsgFunQueue            m_FunQueue;
			TMsgFunQueue            m_high_level_FunQueue; //响应高级别
            INiceNetCounter        *m_pCtMsg;
            boost::recursive_mutex &m_mutex;
			unsigned				m_msg_q_size; //等待处理的消息数目
        };
    }
}
#endif

