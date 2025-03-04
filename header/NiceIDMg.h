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
				purpose:	run�������뱣֤��һ���߳��е��ã�������������ش���
				���æ���� true
			*********************************************************************/
            unsigned        Run(unsigned uMaxCount,unsigned curtime);

			/********************************************************************
			created:	10:11:2010   9:58
			author:		zhanghongtao	
			purpose:	isinserthead,������˲������Ƿ������¼����뵽�¼����е�ͷ��
						Ҳ�����Ƿ����ȴ��������Ϣ��
						����ʹ�õ��¼��У�TCPServer�����������¼�����Ϊ��������ȴ���
						��ʹ�ͻ��˵����ӱ�ķǳ���������Ϊ�󲿷�ʱ������û�м���״̬��
			*********************************************************************/
            void			PutMsgFun(on_netmsg_hand funhand,bool isinserthead=false);
		private:
			unsigned	Run_high_level_FunQueue(unsigned uCount);

        private:
            TMsgFunQueue            m_FunQueue;
			TMsgFunQueue            m_high_level_FunQueue; //��Ӧ�߼���
            INiceNetCounter        *m_pCtMsg;
            boost::recursive_mutex &m_mutex;
			unsigned				m_msg_q_size; //�ȴ��������Ϣ��Ŀ
        };
    }
}
#endif

