/********************************************************************
	created:	2012/01/01
	created:	1:1:2012   13:09
	filename: 	e:\XGameX\XFrame\PeonyNetLib\PeonyNet\header\NetLogMg.h
	file path:	e:\XGameX\XFrame\PeonyNetLib\PeonyNet\header
	file base:	NetLogMg
	file ext:	h
	author:		
	
	purpose:	这个类是被多线程访问的，因为考虑性能和特别的位置和功能
	没有使用锁了同步，因为会严重阻塞性能，所以这个类中间的同时使用的是
	一个糊涂的数据来同步，请不轻易的修改!
*********************************************************************/

#ifndef NetLogMg_h__
#define NetLogMg_h__
#include "./InPublicPreDefine.hpp"
#include "./NiceNetLogicProtocol.h"
using namespace peony::net::INPROTOCOL;

namespace peony
{
	namespace net
	{
		const unsigned sc_logitemcount_max = 1000;
		const unsigned sc_logitembuf_max   = 1024-8;
		struct TLogItem
		{
			unsigned index;
			unsigned datalen;
			unsigned grade;
			bool         isapp;
			unsigned updatetime;
			char		 buffer[sc_logitembuf_max];

		public:
			TLogItem()
			{
				index		= 0;
				datalen		= 0;
				updatetime	= 0;
			}
		};

		struct TLogReader
		{
			unsigned	reader_conid;
			unsigned	appgrade;
			unsigned	netgrade;
			unsigned    lastindex;
		};
		typedef list<TLogReader>LIST_LOGREADER;

		class CNetLogForOpc
		{
		public:
			CNetLogForOpc(void);
			~CNetLogForOpc(void);

			void			Init();
			void			UnInit();
			void			run();
			void			AddLog( unsigned uGrade,string &strkind,stringstream &strIn,bool isnetlog );
			void			AddReader( unsigned conid, const TNiceNetPNetLogOpReaderReq *pMsg );

		private:
			void			ImpAddLog( unsigned uGrade,string &strkind,stringstream &strIn,bool isnetlog );
			bool            RunItem( unsigned CurTime,TLogReader *pReader,unsigned maxcount );
			void            RunGrade();
			TLogReader *	FindReader( unsigned conid );
			void			DelReader( unsigned conid );
			vector<unsigned>  m_vecdelreader;		
		//这些变量被多个线程使用.但修改的只有一个线程
		private:
			unsigned	m_curindex;
			TLogItem	   *m_array[sc_logitemcount_max];
			LIST_LOGREADER  m_readers;

			boost::recursive_mutex m_mutex;

			//为了安全,是否有读者
			bool            m_ishavereaders;

			//为了性能
			unsigned             m_appgrademax;
			unsigned             m_netgrademax;
			TNiceNetPNetLogSendLog   m_tempNetSendMsg;
			unsigned             m_lastrungarde;
		};
	}
}
#endif

