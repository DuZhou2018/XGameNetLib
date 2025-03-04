#ifndef NICEINCOUNTERINSTANCE_NET_IMP
#define NICEINCOUNTERINSTANCE_NET_IMP

#include "../include/INiceNet.h"
#include "../header/NiceNetCounter.h"
#include "./CountersMg.hpp"

/********************************************************************
	created:	2009/11/12	12:11:2009   11:45
	filename: 	e:\zht_PeyoneSolution20090824\PeonyPublic\PeonyNetLib\PeonyNet\header\NetCountersInstanceMg.h
	author:		zhanghongtao    zht213@hotmail.com
	
	purpose:	��������net�ڲ��Լ���ʹ�õļ�����������ʵ��
*********************************************************************/
namespace peony
{
    namespace net
    {
		/********************************************************************
		created:	2009/11/12	12:11:2009   13:10
		author:		zhanghongtao	
		purpose:	��������������ʱ��֪ͨCNetCounterInstanceMg
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
			{//һ�������������һЩ���ܼ�����

				INiceNetCounter			* m_pSConnectCount;       //��ǰ��������
				INiceNetCounter			* m_pSPoolSize;           //��ǰ�Ѿ������˵����ӳصĴ�С

				//������
				INiceNetCounter			* m_pSHttpLSecondCt;      //���һ��Ĳ�����,�����м����µ�����
				INiceNetCounter			* m_pSHttpLMinCt;         //���һ�ֵĲ�����


				//����ͳ��
				INiceNetCounter			* m_pSHttpLSecondFlow;    //���һ�������
				INiceNetCounter			* m_pSHttpLMinFlow;       //���һ�ֵ�����

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

