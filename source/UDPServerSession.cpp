

#include "../header/UDPServerSession.hpp"
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
        //	Implemention of UDPServerSession
        //
		UDPServerSession::UDPServerSession(io_context &io_service,TXZTBaseInitPm &initPM,const TNewServerParam &param)
			:
			m_param(param),
            m_recv_buf(boost::bind(tcpserver_in_alloc_buffer, false, _1, _2, _3, _4), 10240)
        {
			//CAutoLogDeadlock  AutoLgDL( " Connectid=",__FUNCTION__ );
            m_InFunConnect = initPM.mFunConnect;
            m_uXServerID   = initPM.mXServerID;
			m_SockType     = INiceNetSocket::ESockType_Server_Udp;
			strcpy(m_SocketTypeName, "Udp");
            this->SetXMark(socketmk_server_client, 0);
			this->SetXMark(socketmk_IsOpenState);
			this->SetXMark(socketmk_NoCheckHeart);
			m_pSelfAa = INetSocketPtr();
		}
        UDPServerSession::~UDPServerSession()
        {
            NETLOG_ERROR(this->LogSelf()<<FUN_FILE_LINE);
        }
        void UDPServerSession::StartSocket()
        {
            //OpenState();
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
					strcpy(m_selflog," UDPServerSession->start; " );
				}
			}
        }

        bool UDPServerSession::DoNetInCmd(unsigned connectid, const void*pdata, unsigned data_len)
        {
            if( data_len<g_MsgHeadLen )
                return false;
            INPROTOCOL::TInNiceNetHead *pHeadBase = (INPROTOCOL::TInNiceNetHead *)pdata;
            switch ( pHeadBase->GetMsgID() )
            {
			case INPROTOCOL::NiceNetP_NETHEART_NTF:
				{
					INPROTOCOL::TNiceNetTestMsgNtf *pReq = (INPROTOCOL::TNiceNetTestMsgNtf*)pdata;
					if( 12658170 != pReq->iMustValue ){
						INiceNet::Instance().CloseConByID( connectid,"NETHeartMsgIsErr!!!");
					}
					return true;
				}
            default:
                break;
            }
            return false;
        }

		int UDPServerSession::SendMsg(const void* data_ptr, unsigned data_size)
		{
            return 0;
		}

		bool UDPServerSession::Run(unsigned uCount)
		{
            Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
			if(0){//为了调试
				m_recv_buf.pop();
				return true;
			}


			unsigned data_len = 0;
            void    *pBuffer  = m_recv_buf.front(data_len);

            if((data_len>=sc_pak_header_len) && (data_len<=m_recv_buf.get_buffer_maxlen()))
            {
                if(true)
                {
                    if(data_len<g_MsgHeadLen)
                    {
                        NETLOG_ERROR("[消息处理.net] 消息长度异常 lostmsg,"<<LogSelf());
                        return false;
                    }
                }

                recvbuf_lock.unlock();
                try
                {
                    if(!PNGB::IsXMarked(EmZhtCon_NotRunMsg))
                    {
                        m_param.pApp->OnRecive(GetConnectIndex(), pBuffer, data_len);
                    }else{
                        //NETLOG_NORMAL("[消息处理.net] lostmsg,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
                    }
                }catch(...){
                    string strExceptionInfo = CInPubFun::LogExceptionMsgBuf(pBuffer, 6699007);
                    NETLOG_FATAL("[消息处理异常.net.udp2],"<<LogSelf()<<strExceptionInfo<<"; data_len="<<data_len);
                    PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
                }

                recvbuf_lock.lock();
                m_recv_buf.pop();
            }
            else
            {
                NETLOG_FATAL("[消息大小异常，主动关闭连接,net],"<<LogSelf()<<FUN_FILE_LINE);
                this->CloseSocketOut("MsgLenError!");
                return false;
            }
            return true;
		}

        void UDPServerSession::NtfAppConnect( INetSocketPtr pSelf )
        {
			string strAa = boost::str(boost::format("UdpSession_%d")%m_conn_index );
			strcpy(m_selflog, strAa.c_str() );

			m_pSelfAa = pSelf;
            mainrun_call_onconnected(m_uXServerID, this->m_conn_index, XS_Connect_Success, "UdpCreate!!", shared_from_this());
        }

        void UDPServerSession::PushRecOneMsg(char *pBuff, unsigned iDataSize)
        {
            Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
            m_recv_buf.push( pBuff,iDataSize );
			this->zht_CallMRecvHandler();
		}

        void UDPServerSession::CloseSocketOut(string strwhy)
        {
			NETLOG_ERROR("[UdpSession_CloseOut...] " << this->LogSelf() << FUN_LINE);
			try
			{
				mainrun_call_onconnected(m_uXServerID, this->m_conn_index, Both_SelfCloseConnect, "UdpClose!!", shared_from_this());
				m_pSelfAa = INetSocketPtr();
			}
			catch ( ... )
			{
				NETLOG_ERROR("[UdpSession_CloseOut...异常...] " << this->LogSelf() << FUN_LINE);
			}
		}
    }

}	// end of namespace peony
