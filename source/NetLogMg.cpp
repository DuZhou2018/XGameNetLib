#include "../header/NetLogMg.h"
#include "../header/NiceNet.h"
#include "../header/CGlobal.hpp"

namespace peony
{
	namespace net
	{	
		CNetLogForOpc::CNetLogForOpc( void )
		{
			m_ishavereaders    = false;
			m_curindex		   = 0;
			TLogItem *pLogItem = 0;
			for( unsigned t=0; t<sc_logitemcount_max; ++t )
			{
				pLogItem   = new TLogItem();
				m_array[t] = pLogItem;
				memset( pLogItem,0,sizeof(TLogItem) );
				//pLogItem->updatetime = CInPubFun::GetServerRelative();
			}

			m_appgrademax = 0;
			m_netgrademax = 0;

			m_lastrungarde = CInPubFun::GetNowTime();
		}

		CNetLogForOpc::~CNetLogForOpc( void )
		{
			TLogItem *pItem = 0;
			for( unsigned t=0; t<sc_logitemcount_max; ++t )
			{
				pItem = m_array[m_curindex];
				m_array[m_curindex] =0;
				delete pItem;
			}
		}

		void CNetLogForOpc::Init()
		{

		}

		void CNetLogForOpc::UnInit()
		{
			m_readers.clear();
		}

		void CNetLogForOpc::ImpAddLog( unsigned uGrade,string &strkind,stringstream &strIn,bool isnetlog )
		{
			//安全锁
			Boost_Scoped_Lock MsgWritelock(m_mutex);

			unsigned index = m_curindex%sc_logitemcount_max;
			TLogItem *pItem    = m_array[index];
			pItem->index       = m_curindex;

			//写数据一
			unsigned datalen = 0;
			if( strkind.size()+datalen<sc_logitembuf_max)
			{
				memcpy( pItem->buffer+datalen,strkind.c_str(),strkind.size() );
				datalen += (unsigned)strkind.size();
			}else if( datalen<sc_logitembuf_max )
			{
				unsigned copylen = sc_logitembuf_max-pItem->datalen;
				memcpy( pItem->buffer+datalen,strkind.c_str(),copylen );
				datalen += copylen;
			}else
			{
				return;
			}

			//写数据二
			if( strIn.str().size()+datalen<sc_logitembuf_max)
			{
				memcpy( pItem->buffer+datalen,strIn.str().c_str(),strIn.str().size() );
				datalen += (unsigned)strIn.str().size();
			}else if( datalen<sc_logitembuf_max )
			{
				unsigned copylen = sc_logitembuf_max-pItem->datalen;
				memcpy( pItem->buffer+datalen,strIn.str().c_str(),copylen );
				datalen += copylen;
			}else
			{
				return;
			}

			if( datalen>sc_logitembuf_max )
				return;

			pItem->grade      = uGrade;
			pItem->isapp      = !isnetlog;
			pItem->updatetime = PNGB::m_server_relativeTm;
			pItem->datalen    = datalen;
			m_curindex       += 1;
		}

		void CNetLogForOpc::AddLog( unsigned uGrade,string &strkind,stringstream &strIn,bool isnetlog )
		{
			//是否有读者
			if( !m_ishavereaders )
				return;
			if( isnetlog )
			{
				if( 0 == (m_netgrademax & uGrade) )
					return;
			}else
			{
				if( 0 == (m_appgrademax & uGrade) )
					return;
			}

			//还是安全些吧
			if( strkind.size()+100>sc_logitembuf_max )
				return;

			if( strkind.size()+strIn.str().size()+10>sc_logitembuf_max )
			{
				size_t iCopyLen = sc_logitembuf_max-strkind.size()-20;
				char chTemp[sc_logitembuf_max+20]={0};
				memcpy( chTemp,strIn.str().c_str(),iCopyLen );
				stringstream strTempIn;
				strTempIn<<chTemp;
				this->ImpAddLog(uGrade,strkind,strTempIn,isnetlog );
				return;
			}
			else
			{
				this->ImpAddLog(uGrade,strkind,strIn,isnetlog );
			}
		}

		void CNetLogForOpc::run()
		{
			RunGrade();

			while( !m_vecdelreader.empty() )
			{
				vector<unsigned>::iterator it = m_vecdelreader.begin();
				this->DelReader( *it );
				if( m_readers.empty() )
					m_ishavereaders = false;

				m_vecdelreader.erase( it );
			}

			if( m_readers.empty() )
				return;

			unsigned maxloop = 1;
			unsigned CurTime = PNGB::m_server_relativeTm;
			TLogReader *pReader  = 0;
			LIST_LOGREADER::iterator it = m_readers.begin();
			for( ;it!=m_readers.end(); ++it )
			{
				pReader = &(*it);
				maxloop = 1;
				while( maxloop<10 )
				{
					maxloop += 1;
					if( !RunItem( CurTime,pReader,10 ) )
						break;
				}
			}
		}

		//如果返回true，继续发送
		bool CNetLogForOpc::RunItem( unsigned CurTime,TLogReader *pReader,unsigned maxcount )
		{
			static const unsigned sc_netmsgmaxbuffer              = sizeof(m_tempNetSendMsg.buffer);
			static const unsigned sc_netmsgmaxbuffer_fiexpart_len = sizeof(m_tempNetSendMsg)-sizeof(m_tempNetSendMsg.buffer);

			if( true )
			{
				//安全锁,保护pLogItem对象
				Boost_Scoped_Lock MsgWritelock(m_mutex);

				//检查是否有没有发送的log
				if( pReader->lastindex>=m_curindex )
					return false;
				TLogItem *pLogItem = m_array[pReader->lastindex%sc_logitemcount_max];

				//检查时间间隔是允许发送
				if( CurTime-pLogItem->updatetime<5 )
					return false;

				//检查消息长度是否合适
				if( (0==pLogItem->datalen) || (sc_netmsgmaxbuffer<pLogItem->datalen) )
				{
					pReader->lastindex += 1;
					return true;
				}

				//检查定制等级
				if( pLogItem->isapp )
				{
					if( 0==(pLogItem->grade & pReader->appgrade) )
					{
						pReader->lastindex += 1;
						return true;
					}
				}else
				{
					if( 0==(pLogItem->grade & pReader->netgrade) )
					{
						pReader->lastindex += 1;
						return true;
					}
				}

				//都通过.就发送吧
				m_tempNetSendMsg.index    = pLogItem->index;
				m_tempNetSendMsg.datalen  = pLogItem->datalen;
				memcpy( m_tempNetSendMsg.buffer,pLogItem->buffer,pLogItem->datalen );
				m_tempNetSendMsg.SetLen(sc_netmsgmaxbuffer_fiexpart_len+pLogItem->datalen);
				m_tempNetSendMsg.SetNetLogReaderID(pReader->reader_conid);
				pReader->lastindex       += 1;
			}

			//send
			if( !CNiceNet::Instance().SendMsg( pReader->reader_conid,&m_tempNetSendMsg,m_tempNetSendMsg.GetLen()) )
			{
				string strLocalip = CNiceNet::Instance().GetLocalIp( pReader->reader_conid );
				if( strLocalip.empty() )
				{
					m_vecdelreader.push_back( pReader->reader_conid );
					return false;
				}
			}
			return true;
		}

		void CNetLogForOpc::AddReader( unsigned conid, const TNiceNetPNetLogOpReaderReq *pMsg )
		{
			TNiceNetPNetLogOpReaderRep repMsg;
			repMsg.SetLen(sizeof(repMsg));
			repMsg.reqtype  = pMsg->reqtype;
			repMsg.result   = emNiceNetPNetLogOpReaderResult_OK;

			if( emNiceNetPNetLogOpReaderType_Add == pMsg->reqtype )
			{
				TLogReader *pOld = this->FindReader( conid );
				if( pOld )
				{
					pOld->appgrade	= pMsg->appgrade;
					pOld->netgrade	= pMsg->netgrade;
					pOld->lastindex = m_curindex;
				}else
				{
					TLogReader reader;
					reader.reader_conid = conid;
					reader.appgrade     = pMsg->appgrade;
					reader.netgrade     = pMsg->netgrade;
					reader.lastindex    = m_curindex;
					m_readers.push_back( reader );
					m_ishavereaders     = true;
				}

				m_appgrademax |= pMsg->appgrade;
				m_netgrademax |= pMsg->netgrade;
				CNiceNet::Instance().SendMsg( conid,&repMsg,repMsg.GetLen());
				return;
			}
			else if( emNiceNetPNetLogOpReaderType_ModifyGrade == pMsg->reqtype )
			{
				TLogReader *pOld = this->FindReader( conid );
				if( pOld )
				{
					pOld->appgrade	= pMsg->appgrade;
					pOld->netgrade	= pMsg->netgrade;
					pOld->lastindex = m_curindex;
					m_vecdelreader.push_back( conid );
				}else
				{
					repMsg.result   = emNiceNetPNetLogOpReaderResult_Fail;
				}
				CNiceNet::Instance().SendMsg( conid,&repMsg,repMsg.GetLen());
			}
			else if( emNiceNetPNetLogOpReaderType_Del == pMsg->reqtype )
			{
				TLogReader *pOld = this->FindReader( conid );
				if( pOld )
				{
					pOld->appgrade	= 0;
					pOld->netgrade	= 0;
					pOld->lastindex = m_curindex;
					m_vecdelreader.push_back( conid );
				}else
				{
					repMsg.result   = emNiceNetPNetLogOpReaderResult_Fail;
				}
				CNiceNet::Instance().SendMsg( conid,&repMsg,repMsg.GetLen());
			}
		}

		void CNetLogForOpc::DelReader( unsigned conid )
		{
			TLogReader *pReader = 0;
			LIST_LOGREADER::iterator it = m_readers.begin();
			for( ;it!=m_readers.end(); ++it )
			{
				pReader = &(*it);
				if( pReader->reader_conid == conid )
				{
					m_readers.erase( it );
					return;
				}
			}
		}

		TLogReader * CNetLogForOpc::FindReader( unsigned conid )
		{
			TLogReader *pReader = 0;
			LIST_LOGREADER::iterator it = m_readers.begin();
			for( ;it!=m_readers.end(); ++it )
			{
				pReader = &(*it);
				if( pReader->reader_conid == conid )
				{
					return pReader;
				}
			}
			return 0;
		}

		void CNetLogForOpc::RunGrade()
		{
			if( CInPubFun::GetNowTime()-m_lastrungarde<10 )
				return;

			unsigned agrade = 0;
			unsigned ngrade = 0;
			TLogReader *pReader = 0;
			LIST_LOGREADER::iterator it = m_readers.begin();
			for( ;it!=m_readers.end(); ++it )
			{
				pReader = &(*it);
				agrade |= pReader->appgrade;
				ngrade |= pReader->netgrade;
			}
			m_appgrademax = agrade;
			m_netgrademax = ngrade;
		}
	}
}