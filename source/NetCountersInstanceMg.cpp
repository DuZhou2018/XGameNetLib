#include "../header/NetCountersInstanceMg.h"
#include "../header/NiceNet.h"
#include "../header/CGlobal.hpp"
#include "../header/INiceNetServer.h"
#include "../header/ClientServerIDMG.h"
//#include <boost/format.hpp>

namespace peony
{
    namespace net
    {
		CNetCounterInstanceMg::CNetCounterInstanceMg()/*:m_mutex(*PNGB::m_globMutex)*/
		{
			//m_con_timer.restart();
		}		

		CNetCounterInstanceMg::~CNetCounterInstanceMg()
		{
		}
		void CNetCounterInstanceMg::AddOrDelServer(bool isHttp,unsigned uServerConID,bool isadd)
		{
			Map_ServerCounters::iterator it = m_ServerS.find(uServerConID);
			if( isadd )
			{
				if( it != m_ServerS.end() )
				{
					NETLOG_ERROR("This counter exist:uServerConID="<<uServerConID<<FUN_FILE_LINE);
					return;
				}
				string strPath = str( boost::format("Peony::TcpServer::server_%4d")%uServerConID );
				string strDesc = str( boost::format("server connected count,instanceid:%d")%uServerConID );
				if( isHttp ){
					strPath = str( boost::format("Peony::HttpServer::server_%4d")%uServerConID );
				}

				TServerCounters server;
				server.m_pSConnectCount		= INiceNetCounter::CreateCounter(strPath.c_str(),"ConnectSize", strDesc.c_str() );
				server.m_pSPoolSize         = INiceNetCounter::CreateCounter(strPath.c_str(),"SessPoolSize",strDesc.c_str() );
				server.m_pSHttpLSecondCt	= INiceNetCounter::CreateCounter(strPath.c_str(),"LastSecCt","ThisServer last second connect count!" );
				server.m_pSHttpLMinCt		= INiceNetCounter::CreateCounter(strPath.c_str(),"LastMinCt","ThisServer last minute connect count!" );
				server.m_pSHttpLSecondFlow	= INiceNetCounter::CreateCounter(strPath.c_str(),"LastSecFlow","ThisServer last second happen flow (k)" );
				server.m_pSHttpLMinFlow		= INiceNetCounter::CreateCounter(strPath.c_str(),"LastMinFlow","ThisServer last minute happen flow (k)");
				m_ServerS[uServerConID] = server;
			}else
			{
				if( it != m_ServerS.end() )
				{
					INiceNetCounter::DeleteCounter( it->second.m_pSConnectCount );
					INiceNetCounter::DeleteCounter( it->second.m_pSPoolSize );
					INiceNetCounter::DeleteCounter( it->second.m_pSHttpLSecondCt );
					INiceNetCounter::DeleteCounter( it->second.m_pSHttpLMinCt );
					INiceNetCounter::DeleteCounter( it->second.m_pSHttpLSecondFlow );
					INiceNetCounter::DeleteCounter( it->second.m_pSHttpLMinFlow );
					m_ServerS.erase( uServerConID );
				}
			}
		}

		bool CNetCounterInstanceMg::UpdateSessionCounterValue(unsigned uConID,CNetCounterInstanceMg::TSessionCounters *pSSC)
		{
			TSessionBufferInfo ssinfo;
			memset(&ssinfo,0,sizeof(ssinfo) );
			if( INiceNet::Instance().GetSessionBufferInfo( uConID,ssinfo ) )
			{
				pSSC->pSendOK->UpdateReplace( ssinfo.uSendOKCount );
				pSSC->pSendFail->UpdateReplace(ssinfo.uSendFailCount );
				pSSC->pSendPercent->UpdateReplace(ssinfo.uSendUsedPercent );
				pSSC->pRecive->UpdateReplace( ssinfo.uReciveCount );
				pSSC->pRecivePercent->UpdateReplace( ssinfo.uReciveUsedPercent );
				return true;
			}
			return false;
		}

		void CNetCounterInstanceMg::OnInCounterTime()
		{
			if( m_con_timer.elapsed()<1.0f )
				return;
			m_con_timer.restart();

			Map_ServerCounters::iterator it = m_ServerS.begin();
			for( ;it!=m_ServerS.end(); ++ it )
			{
				TServerCounters &ServerCt = it->second;
				INiceNetServer *pserver = CNiceNet::Instance().FindServer( it->first );
				if( pserver )
				{
					ServerCt.m_pSConnectCount->UpdateReplace( pserver->get_conn_count() );
					ServerCt.m_pSPoolSize->UpdateReplace( pserver->GetCurSessionsPoolSize() );

					TStatData *pStat = pserver->GetStatData();
					ServerCt.m_pSHttpLSecondCt->UpdateReplace( pStat->LastSecondCount.uValue );
					ServerCt.m_pSHttpLMinCt->UpdateReplace( pStat->LastMinuteCount.uValue );
					ServerCt.m_pSHttpLSecondFlow->UpdateReplace( pStat->RecLastSecFlow.uValue/1024 );
					ServerCt.m_pSHttpLMinFlow->UpdateReplace( pStat->RecLastMinFlow.uValue/1024 );

					pStat->Reset( PNGB::m_server_curTm );
				}
			}

			//connect
			unsigned SessConid = 0;
			Map_SessionCounters::iterator itss = m_SessionCt.begin();
			for( ;itss!=m_SessionCt.end(); ++itss )
			{
				SessConid                   = itss->first;
				TSessionCounters &SessionCt = itss->second;
				if( !UpdateSessionCounterValue( SessConid, &SessionCt ) )
				{
					this->DelTcpSession( &SessionCt );
					m_SessionCt.erase( itss );
					break;
				}
			}
		}

		void CNetCounterInstanceMg::AddTcpSession( unsigned conid )
		{
			Map_SessionCounters::iterator itss = m_SessionCt.find( conid );
			if( itss!=m_SessionCt.end() )
				return;

			string strRemoIp = INiceNet::Instance().GetRemoteIp( conid );
			string strPath   = str( boost::format("Peony::TcpCt::conid_%d")%conid );
			TSessionCounters tcp_st_ct;
			if(1)
			{
				string strDesc    = str( boost::format("pRecive,conid:%d,ip=%s")%conid%strRemoIp );
				string strName    = str( boost::format("pRecive") );
				tcp_st_ct.pRecive = INiceNetCounter::CreateCounter(strPath.c_str(),strName.c_str(),strDesc.c_str() );
			}
			if(1)
			{
				string strDesc			 = str( boost::format("pRecivePercent,conid:%d,ip=%s")%conid%strRemoIp );
				string strName           = str( boost::format("pRecivePercent") );
				tcp_st_ct.pRecivePercent = INiceNetCounter::CreateCounter(strPath.c_str(),strName.c_str(),strDesc.c_str() );
			}
			if(1)
			{
				string strDesc			 = str( boost::format("pSendFail,conid:%d,ip=%s")%conid%strRemoIp );
				string strName           = str( boost::format("pSendFail") );
				tcp_st_ct.pSendFail		 = INiceNetCounter::CreateCounter(strPath.c_str(),strName.c_str(),strDesc.c_str() );
			}
			if(1)
			{
				string strDesc			 = str( boost::format("pSendOK,conid:%d,ip=%s")%conid%strRemoIp );
				string strName           = str( boost::format("pSendOK") );
				tcp_st_ct.pSendOK		 = INiceNetCounter::CreateCounter(strPath.c_str(),strName.c_str(),strDesc.c_str() );
			}
			if(1)
			{
				string strDesc			 = str( boost::format("pSendPercent,conid:%d,ip=%s")%conid%strRemoIp );
				string strName           = str( boost::format("pSendPercent") );
				tcp_st_ct.pSendPercent	 = INiceNetCounter::CreateCounter(strPath.c_str(),strName.c_str(),strDesc.c_str() );
			}

			if( tcp_st_ct.pRecive        &&
				tcp_st_ct.pRecivePercent &&
				tcp_st_ct.pSendFail      &&
				tcp_st_ct.pSendOK        &&
				tcp_st_ct.pSendPercent )
			{
				m_SessionCt[conid]       = tcp_st_ct;
			}
			else
			{
				INiceNetCounter::DeleteCounter( tcp_st_ct.pRecive			);
				INiceNetCounter::DeleteCounter( tcp_st_ct.pRecivePercent	);
				INiceNetCounter::DeleteCounter( tcp_st_ct.pSendFail			);
				INiceNetCounter::DeleteCounter( tcp_st_ct.pSendOK			);
				INiceNetCounter::DeleteCounter( tcp_st_ct.pSendPercent		);
			}
		}

		void CNetCounterInstanceMg::DelTcpSession( unsigned conid )
		{
			Map_SessionCounters::iterator itss = m_SessionCt.find( conid );
			if( itss!=m_SessionCt.end() )
			{
				TSessionCounters &SessionCt = itss->second;
				this->DelTcpSession( &SessionCt );
				m_SessionCt.erase( itss );
			}
		}

		void CNetCounterInstanceMg::DelTcpSession( TSessionCounters *pSSC )
		{
			if( !pSSC )
				return;
			INiceNetCounter::DeleteCounter( pSSC->pRecive			);
			INiceNetCounter::DeleteCounter( pSSC->pRecivePercent	);
			INiceNetCounter::DeleteCounter( pSSC->pSendFail			);
			INiceNetCounter::DeleteCounter( pSSC->pSendOK			);
			INiceNetCounter::DeleteCounter( pSSC->pSendPercent		);
		}
	}
}


