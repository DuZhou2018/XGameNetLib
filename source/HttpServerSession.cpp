

#include "../header/HttpServerSession.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CountersMg.hpp"
#include "../header/NiceNet.h"
#include "../header/CGlobal.hpp"
#include "../header/CountersMg.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/NetLogMg.h"

namespace peony 
{
    namespace net 
    {
        //
        //	Implemention of HttpServerSession
        //
        HttpServerSession::~HttpServerSession()
        {
        }

		HttpServerSession::HttpServerSession(io_context &io_service, TXZTBaseInitPm &initPM,
			const TNewServerParam &param)
		   :XTcpSocketBase(io_service,initPM ),
			m_param(param)
		{
			strcpy(m_SocketTypeName, "Http");
			this->SetXMark( socketmk_server_client,0 );
			m_SockType = INiceNetSocket::ESockType_Server_Http;
		}

		void HttpServerSession::StartSocket()
        {
            OpenState();

			//self log
			if( 1 )
			{
				unsigned uRport;
				string strRomeIP = this->GetRemoteIp( uRport );
				unsigned uLocalport;
				string strLocalIP = this->GetLocalIp( uLocalport );

				ostringstream strselflog;
				strselflog<<" [ServerSession:"<<GetConnectIndex()<<"] Local["<<strLocalIP<<","<<uLocalport<<"] Remote["<<strRomeIP<<","<<uRport<<"]";
				if( strselflog.str().size()<sizeof(m_selflog) )
				{
					memset( m_selflog,0,sizeof(m_selflog) );
					strcpy(m_selflog,strselflog.str().c_str() );
				}
				else
				{
					memset( m_selflog,0,sizeof(m_selflog) );
					strcpy(m_selflog," HttpServerSession->start; " );
				}
				//NETLOG_NORMAL( m_selflog<<FUN_FILE_LINE );
			}
        }

		bool HttpServerSession::XxTcpBeginRecWork()
		{
			read_OneHttpReq();
			return true;
		}

		bool HttpServerSession::DoNetInCmd(unsigned connectid, IHttpReq *pHttpReq )
        {
			string strFun = pHttpReq->GetMethod();
			if( strFun.empty() )
				return false;

			m_pStatData->RecLastSecFlow.uValue += pHttpReq->Stat_GetRecSize();
			m_pStatData->RecLastMinFlow.uValue += pHttpReq->Stat_GetRecSize();

			if( string("/PingPNet") == strFun )
			{
				unsigned uPost=0;
				string strRip = INiceNet::Instance().GetRemoteIp( uPost,connectid );
				string strFKey ="";
				string strFParam = pHttpReq->GetParamByIndex( 0,strFKey );
				ostringstream strRep;
				strRep<<":<br />";
				strRep<<":PingPNet CurServerTime="<<CInPubFun::GetNowTime()<<" <br />";
				strRep<<":NetWorkThreadID="<<pHttpReq->Debug_GetNThreadID()<<" <br />";
				strRep<<":IeHost="<<strRip<<":"<<uPost<<"<br />";
				strRep<<":ParamCt="<<pHttpReq->GetParamCount()<<"; FParam="<<strFKey<<"="<<strFParam<<"<br />";
				pHttpReq->RepMsg( strRep.str().c_str(),(unsigned)strRep.str().size() );
				return true;
			}else if( string("/")==strFun )
			{
				string strTempAa = boost::str( boost::format("ThisPNetRepMsg: curtime:%u!")%CInPubFun::GetNowTime() );
				pHttpReq->RepMsg( strTempAa.c_str(),(unsigned)strTempAa.size() );
				return true;
			}
            return false;
        }

		int HttpServerSession::SendMsg(const void* data_ptr, size_t data_size)
		{
			Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
			bool IsNeedCloseSocket = false;
			int iErrID = SubCheckTcpIsMaySendMsg( m_send_buf, data_ptr, (unsigned)data_size, IsNeedCloseSocket);
			if (iErrID > 0)
			{
				NETLOG_ERROR("[����ʧ��,��Ҫ�ر�, net��]. wirte<1>!  iErrID=" << iErrID << LogSelf() << FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				if (IsNeedCloseSocket)
				{
					CloseSocketOut("SendMsgErrorXx!");
					return 9090106;
				}
				return iErrID;
			}

			unsigned uData_len = 0;
			void *pSend = m_send_buf.front(uData_len);
			char *pHttpSend = (char*)pSend;
			pHttpSend += sc_pak_header_len;
			uData_len -= sc_pak_header_len;
			iErrID = this->XBase_WriteData( pHttpSend,uData_len,uData_len,0 );
			if( iErrID>0 )
			{
				CloseSocketOut("SendMsgException1!");
				return 9070106;
			}

			m_pStatData->SendLastMinFlow.uValue += (unsigned)data_size;
			m_pStatData->SendLastMinFlow.uValue += (unsigned)data_size;
			return 0;
		}

		bool HttpServerSession::Run(unsigned uCount)
		{
			/*
				������������̵߳��õģ�ʹ�õ�����˳��
				nicenetģ���locker.
				��Ϣ��Դ��socket��recivelocker��
				��Ϣ��Դsocket��senderlocker/���socket��senderlocker
			*/

			//CAutoLogDeadlock AutoLgDL(__FUNCTION__);
			Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);

			try{
				if( !DoNetInCmd(GetConnectIndex(),&m_OneReq ) )
				{
					if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
					{
						m_param.pApp->OnRecHttp( GetConnectIndex(),&m_OneReq );
					}else
					{
					}
				}
			}
			catch(...){
				NETLOG_FATAL("[��Ϣ�����쳣.net.http],"<<LogSelf()<<" FunName="<<m_OneReq.GetMethod() );	
				//PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
			}

			return true;
		}

		void HttpServerSession::XBase_ReadCb(const ZBoostErrCode& error,size_t bytes_transferred,int iDataType)
		{
			if(error)
			{
				if( IsXMark(socketmk_log) )
				{
					string strerrormsg = error.message();
					NETLOG_ERROR( LogSelf()<<", message=["<<error.value()<<" ; "<<error.message()<<"]  "<<FUN_LINE );
				}else{
					//string strerrormsg = error.message();
					//NETLOG_NORMAL( LogSelf()<<", message=["<<error.value()<<" ; "<<error.message()<<"]  "<<FUN_LINE );
				}

				inclose("HanleReadFail!");
				ZBoostErrCode ec;
				m_socket.close(ec);
				return;
			}
			m_LastLiveTm = PNGB::m_server_curTm;

			//NETLOG_SYSINFO("[�յ�����]"<<LogSelf()<<";bytes_size="<<bytes_transferred<<FUN_LINE );
			//m_recv_buf.m_RHeader.Len = ������ͷ+���Ͻ��յ�����Ϣ����
			unsigned uTempLen = m_recv_buf.m_RHeader.GetLen();
			uTempLen         += (unsigned)bytes_transferred;
			m_recv_buf.m_RHeader.SetLen( uTempLen );

			//��������������Ŀ�ʼλ��
			EmHttpJudge iresult = m_OneReq.OnRevMsg( m_recv_buf.m_RHeader );
			if( m_OneReq.GetPostBodyLen()+2048>m_recv_buf.get_buffer_maxlen() )
			{
				CloseSocketOut("PostBodyTooMax!");
				return;
			}

			if( EHttpJ_ContinueRead ==  iresult )
			{//����������
				m_recv_buf.finish_push_http( (unsigned)bytes_transferred, false );
				read_OneHttpReq( m_OneReq.GetBodyNoReadLen() );
			}else if( EHttpJ_OneReqOver == iresult )
			{//������һ������
				m_recv_buf.finish_push_http( (unsigned)bytes_transferred, true );
				m_OneReq.OneReqOver( m_recv_buf.m_RHeader );
				this->zht_CallMRecvHandler();

				//����ǳ����ӣ������Ȳ�ʹ��
				m_recv_buf.m_RHeader.reset();
				read_OneHttpReq();
			}else
			{//�����˴���
				CloseSocketOut("LogicReadDataFail!");
				return;
			}
		}

		void HttpServerSession::XBase_WriteCb(const ZBoostErrCode& error,int iDataType)
		{
			if( !error )
			{
				Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);

				// pop one pak has been sent
				m_send_buf.pop();

				// get next
				unsigned uData_len = 0;
				void *pSend = m_send_buf.front(uData_len);
				if( uData_len<=0 )
				{
					CloseSocketOut("HttpRepSendOver..");
					return;
				}

				bool ishappend_error		= false;
				tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
				if( ((unsigned)ppak_header->GetLen()) > m_send_buf.get_buffer_maxlen())
				{
					//���գ����ĳ��ȱ��������������󣬺����԰�ͷ����
					ishappend_error = true;
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] ��ͷ̫��! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_LINE);
				}

				if( ppak_header->GetLen()<sc_pak_header_len )
				{
					//���գ����ĳ��ȱȰ�ͷ��С�����Դ���
					ishappend_error = true;
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] ��ͷ̫С[<12]! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_LINE);
				}

				if( !ppak_header->check() )
				{
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<";CheckCode="<<ppak_header->getCheckCode()<<FUN_LINE);
					ishappend_error = true;
				}

				if( ishappend_error )
				{
					CloseSocketOut("HandleWriteLogicError!");
					return;
				}
				m_LastLiveTm = PNGB::m_server_curTm;

				char *pHttpSend = (char*)pSend;
				pHttpSend += sc_pak_header_len;
				uData_len -= sc_pak_header_len;
				int iErrID = this->XBase_WriteData( pHttpSend,uData_len,uData_len,0 );
				if( iErrID>0 )
				{
					CloseSocketOut("SendMsgException!");
				}

				if( PNGB::m_pLog->IsMayLog(Log_DebugNet) )
				{
					if( IsXMark(socketmk_server_client) && IsXMark(socketmk_log) )
					{
						NETLOG_DBNET("[SendPack]; ConID="<<GetConnectIndex()<<";SendPack["<<ppak_header->GetMsgID()<<";"<<ppak_header->GetLen()<<";"<<"]");
					}
				}
			}
			else
			{
				if( IsXMark(socketmk_log) )
				{
					NETLOG_ERROR(LogSelf()<<", error="<<error.message()<<",errorid="<<error.value()<<FUN_LINE );
				}

				CloseSocketOut("HandleWriteErrorAa!");
				ZBoostErrCode ec;
				m_socket.close(ec);
				return;
			}

		}

		bool HttpServerSession::read_OneHttpReq(unsigned uReadLen/*=0*/)
		{
			unsigned sRecLen = 1024;
			if( 0==uReadLen )
			{
				sRecLen = 1024;
			}else
			{
				sRecLen = uReadLen-uReadLen%1024;
				if( sRecLen>4096 )
					sRecLen = 4096;
			}

			char *pBuffer = (char*)m_recv_buf.open_for_push( sRecLen );
			if( 0==pBuffer )
			{
				NETLOG_ERROR("[net.���ջ�����̫С] sRecLen="<<sRecLen<<LogSelf()<<FUN_LINE);
				return false;
			}
			if( 0==m_recv_buf.m_RHeader.GetLen() )
			{
				m_OneReq.OneReqBegin( m_conn_index,pBuffer );
				m_recv_buf.m_RHeader = m_OneReq.GetMsgHead();

				//��һ��д��ͷ�����滹Ҫ��дһ��
				memcpy( pBuffer,&(m_recv_buf.m_RHeader),sc_pak_header_len );
				m_recv_buf.finish_push_http( (unsigned)sc_pak_header_len,false );

				pBuffer += sc_pak_header_len;
			}

			size_t sReadLen = sRecLen-32-sc_pak_header_len;
			int iErrID = this->XBase_ReadData( pBuffer,(unsigned)sReadLen,1,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[net.�쳣..1]"<<LogSelf()<<FUN_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("ReadMsgBodyException!");
				return false;
			}
			return true;
		}

    }	// end of namespace net

}	// end of namespace peony
