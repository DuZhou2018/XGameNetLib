#ifndef NICEINCOUNTERINSTANCE_NET_IMP
#define NICEINCOUNTERINSTANCE_NET_IMP

#include "../include/INiceNet.h"
#include "../header/NiceNetCounter.h"
#include "./CountersMg.hpp"

/********************************************************************
	created:	2009/11/12	12:11:2009   11:45
	filename: 	e:\zht_PeyoneSolution20090824\PeonyPublic\PeonyNetLib\PeonyNet\header\NetCountersInstanceMg.h
	author:		zhanghongtao    zht213@hotmail.com
	
	purpose:	用来管理net内部自己所使用的计数器服务器实例
*********************************************************************/
namespace peony
{
    namespace net
    {
		/********************************************************************
		created:	2009/11/12	12:11:2009   13:10
		author:		zhanghongtao	
		purpose:	当有新连接来的时候通知CNetCounterInstanceMg
		*********************************************************************/
        class CNetCounterInstanceMg
        {
			struct TSessionCounters
			{
				INiceNetCounter * pSendOK;
				INiceNetCounter * pSendFail;
				INiceNetCounter * pSendPercent;
				INiceNetCounter * pRecive;
				INiceNetCounter * pRecivePercent;

				TSessionCounters()
				{
					pSendOK			= 0;
					pSendFail		= 0;
					pSendPercent	= 0;
					pRecive			= 0;
					pRecivePercent	= 0;

				}
			};
			typedef map<unsigned,TSessionCounters> Map_SessionCounters;
				
			struct TServerCounters
			{//一个服务器最近的一些性能计数器

				INiceNetCounter			* m_pSConnectCount;       //当前的连接数
				INiceNetCounter			* m_pSPoolSize;           //当前已经分配了的连接池的大小

				//并发数
				INiceNetCounter			* m_pSHttpLSecondCt;      //最近一秒的并发数,就是有几个新的连接
				INiceNetCounter			* m_pSHttpLMinCt;         //最近一分的并发数


				//流量统计
				INiceNetCounter			* m_pSHttpLSecondFlow;    //最近一秒的流量
				INiceNetCounter			* m_pSHttpLMinFlow;       //最近一分的流量

				TServerCounters()
				{
					m_pSConnectCount	= 0;
					m_pSHttpLSecondCt	= 0;
					m_pSHttpLMinCt		= 0;
					m_pSHttpLSecondFlow = 0;
					m_pSHttpLMinFlow	= 0;
					m_pSPoolSize        = 0;
				}
			};
			typedef map<unsigned,TServerCounters> Map_ServerCounters;
        protected:
			friend class PNGB;
            CNetCounterInstanceMg(void);
		public:
            ~CNetCounterInstanceMg(void);

			void AddOrDelServer(bool isHttp,unsigned uServerConID,bool isadd );
            void OnInCounterTime();

			void AddTcpSession( unsigned conid );
			void DelTcpSession( unsigned conid );

		protected:
			void DelTcpSession( TSessionCounters *pSSC );
			bool UpdateSessionCounterValue(unsigned uConID,TSessionCounters *pSSC);

		private:
			Map_ServerCounters		m_ServerS;
			Map_SessionCounters     m_SessionCt;
			CZNetTimer    			m_con_timer;
        };
    }
}
#endif

