#include "../header/INiceNetServer.h"
#include "../header/CAllocMg.h"
#include "../header/NiceIDMg.h"
#include "../header/CGlobal.hpp"
#include "../header/Scheduler.hpp"
#include "../header/Scheduler_impl.hpp"
#include "../include/NiceNetDefine.h"

namespace peony
{
	namespace net 
	{
		INiceNetServer::INiceNetServer():
			m_io_service(PNGB::m_pScheduler->get_impl()->get_ioservice()),
			m_strand(PNGB::m_pScheduler->get_impl()->get_strand())
        {
			m_ListenCon.isStop=false;
			//CZhtBianyiOk g_BIanyiZhtOK;
		}

		INiceNetServer::~INiceNetServer()
		{

		}

		bool tcpserver_in_alloc_buffer(bool using_alloc_sys,char *&buffer,unsigned usize,bool is_req_alloc,unsigned umark)
		{
			if( using_alloc_sys )
			{
				if( is_req_alloc )
				{
					return PNGB::m_pAllocMg->alloc_buffer( buffer,usize );
				}else
				{
					if( false == PNGB::m_pAllocMg->free_buffer( buffer,usize ) )
					{
						//NETLOG_ERROR("usize="<<usize<<",umark="<<umark<<FUN_FILE_LINE);
						return false;
					}
					return true;
				}
			}else
			{
				if( is_req_alloc )
				{
					buffer = new (std::nothrow) char[usize];
                    memset( buffer,0,usize );
					return (0!=buffer);
				}else
				{
					if( buffer )
					{
						delete []buffer;
						buffer =0;
					}
				}
				return true;
			}
			return false;
		}
		void ZhtMsgFun_server_connect(INetSocketPtr psession,unsigned uServerID,unsigned conindex,EmConnectInfo status,string strInfo)
		{
			int istatus = -213;
			if( psession->SockType_IsClient() )
			{
				switch(status)
				{
				case Both_SelfCloseConnect:
					{
						istatus = IAppClient::sc_status_connectclose;
						break;
					}
				case CI_Connect_Success:
					{
						istatus = IAppClient::sc_status_connectok;
						break;
					}
				case CI_CONNECTION_FAILED:
					{
						istatus = IAppClient::sc_status_connectfail;
						break;
					}
				case CI_ADDR_RESOVLE_FAILED:
					{
						istatus = IAppClient::sc_status_gethostfail;
						break;
					}
				default: //CI_ADDR_RESOVLE_SUCCESSED
					return;
				}
			}else
			{
				if( XS_Connect_Success==status ){
					istatus = IAppServer::sc_server_status_ok;
				}else if( XS_MeIsOpcTool==status ){
					istatus = IAppServer::sc_server_status_isopctool;
				}else{//XS_Connect_Cut
					istatus = IAppServer::sc_server_status_close;
				}
			}

			try
			{
				psession->OnConnect_CallXApp(uServerID,conindex,istatus,strInfo);
			}catch (...)
			{
				//NETLOG_ERROR("[异常.net], uServerID="<<uServerID<<", conindex="<<conindex<<", status="<<status<<FUN_FILE_LINE);
			}
		}
		void mainrun_call_onconnected(unsigned uServerID,unsigned connindex,EmConnectInfo status,string info,INetSocketPtr pAutoPtrSession)
		{
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(ZhtMsgFun_server_connect,pAutoPtrSession,uServerID,connindex,status,info) );		
		}

		void ZhtMsgFun_server_onrecv(INetSocketPtr psession)
		{
			psession->Run(1);
		}
		void mainrun_call_onrecvmsg(INetSocketPtr pAutoPtrSession )
		{//call_onrecv_handler
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(ZhtMsgFun_server_onrecv,pAutoPtrSession) );
		}
		
		void mainrun_call_newtcpsocket(INiceNetServer *pself,INetSocketPtr session_ptr,bool isok)
		{
			if( PNGB::m_isbegin_closesys )
				return;
			string strRemoteIp = session_ptr->GetRemoteIp();
            int iErrID = pself->is_may_accept_newconnect(strRemoteIp);
			if( isok && (0==iErrID) )
    		{//充数目上判读是否可以接收新的连接
				mainrun_call_onconnected( pself->GetID(),session_ptr->GetConnectIndex(),
					                         XS_Connect_Success,"NewServerSessionOK",session_ptr );
				session_ptr->StartSocket();
				if( session_ptr->IsXMark(socketmk_log) || session_ptr->IsXMark(socketmk_logic_iniserver) )
				{
					//unsigned ureport = 0;
					//string remoteip      = session_ptr->GetRemoteIp( ureport );
					//NETLOG_SYSINFO("new connect,Server:"<<pself->GetID()<<";accept "<<session_ptr->GetConnectIndex()<<"; remote_ip="<<remoteip<<"; remote_port="<<ureport);
				}
			}else
			{
                string strWhy = boost::str(boost::format("CurTcpConnectCount have max, NoAcceptNewCon; iErrID=%d")%iErrID);
				session_ptr->CloseSocketOut( strWhy );
				session_ptr.reset();
			}
		}

        void INiceNetSocket::OnConnect_CallXApp(unsigned uMyServerID, unsigned uConnectID, int status, std::string info)
        {
            m_InFunConnect(uMyServerID, uConnectID, (EmConnectInfo)status, info);
        }

        ////@@@@@@@@@@@@@@@@@@@@@@@@

        void INiceNetSocket::GetNewConID()
        {
            unsigned index = PNGB::m_pCSIDMG->GetNewID(IDKIND_SERVER_SUBCLIENT, m_uXServerID);
            if((index == INVALID_CONN_INDEX) && (0==index))
            {
                NETLOG_FATAL("[得到新的连接ID失败!]"<<FUN_FILE_LINE);
            }
            m_conn_index = index;
        }

        void INiceNetSocket::FreeConID()
        {
            PNGB::m_pCSIDMG->FreeID(m_conn_index);
            m_conn_index = INVALID_CONN_INDEX;
        }

        bool INiceNetSocket::CheckHeartTimeout(unsigned uMaxTime)
        {
            if(0==uMaxTime)
                return false;
            if(!this->IsXMark(socketmk_IsOpenState))
                return false;
            if(!this->IsXMark(socketmk_NoCheckHeart))
                return false;
            return (PNGB::m_server_curTm-m_LastLiveTm)>uMaxTime;
        }

		void INiceNetSocket::zht_CallMRecvHandler()
		{
			mainrun_call_onrecvmsg(boost::enable_shared_from_this<INiceNetSocket>::shared_from_this());
		}

		peony::net::tcp_pak_header * INiceNetSocket::SendNextCheckSendMsgHead(SendBufferType &send_buf, bool &IsHasErr)
		{
			IsHasErr = false;
			//get next
			unsigned uData_len = 0;
			void *pSend = send_buf.front(uData_len);
			if (uData_len <= 0)
				return 0;

			bool ishappend_error = false;
			tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
			if (((unsigned)ppak_header->GetLen()) > send_buf.get_buffer_maxlen())
			{
				//我日，包的长度比整个缓冲区还大，很明显包头错误
				IsHasErr = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接] 包头太长! pHeader->Len=" << ppak_header->GetLen() << LogSelf() << FUN_FILE_LINE);
				return 0;
			}

			if (ppak_header->GetLen() < sc_pak_header_len)
			{
				//我日，包的长度比包头还小，明显错误
				IsHasErr = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接] 包头太小[<12]! pHeader->Len=" << ppak_header->GetLen() << LogSelf() << FUN_FILE_LINE);
				return 0;
			}

			if (!ppak_header->check())
			{
				NETLOG_ERROR("[数据包头错误,将关闭连接] pHeader->Len=" << ppak_header->GetLen() << LogSelf() << ";CheckCode=" << ppak_header->getCheckCode() << FUN_FILE_LINE);
				IsHasErr = true;
				return 0;
			}
			return ppak_header;
		}

		int INiceNetSocket::SubCheckUdpIsMaySendMsg(SendBufferType &send_buf, const void* data_ptr, unsigned data_size, bool &IsCloseSocket)
		{
			if (data_size >= send_buf.get_buffer_maxlen())
				return 9080101;

			bool send_buf_is_empty = send_buf.IsEmpty();
			if (!send_buf.push(data_ptr, (unsigned)data_size))
			{
				NETLOG_ERROR(LogSelf() << "缓冲区满,发送失败,connecid=" << m_conn_index << ";" << send_buf.get_buffer_info());
				if (this->IsXMark(socketmk_isclose_sendfail))
				{
					//如果是客户端的链接
					if (this->IsXMark(socketmk_server_client) /*&& (0==m_conn_index%50)*/)
					{
						vector<tcp_pak_header> vecmsgheads;
						send_buf.debug_print_buffermsglist(m_conn_index, vecmsgheads);
						//WriteNetBufferMsgListInfo(m_conn_index,vecmsgheads );
					}

					NETLOG_ERROR("[net.udpClientSession.send]关闭连接,缓冲区满,发送失败;connecid:" << m_conn_index << "; " << send_buf.get_buffer_info());
					CloseSocketOut("SendBufFill!");
				}
				return 9080103;
			}

			m_pStatData->SendLastSecFlow.uValue += (unsigned)data_size;
			m_pStatData->SendLastMinFlow.uValue += (unsigned)data_size;
			return SubCheckSendMsgHead(send_buf);
		}

		int INiceNetSocket::SubCheckTcpIsMaySendMsg(SendBufferType &send_buf, const void* data_ptr, unsigned data_size, bool &IsCloseSocket)
		{
			TcpSocket *pSocket = GetTcpSocket();
			IsCloseSocket = false;
			if (false == IsGoodConnect())
				return 9070101;
			if (!pSocket->is_open())
			{
				NETLOG_ERROR("[Socket is not open]," << LogSelf() << FUN_LINE);
				return 9070102;
			}

			if (data_size >= send_buf.get_buffer_maxlen())
				return 9070103;

			IsCloseSocket = true;
			//bool send_buf_is_empty = send_buf.IsEmpty();
			if (!send_buf.push(data_ptr, (unsigned)data_size))
			{
				NETLOG_ERROR(LogSelf() << "缓冲区满,发送失败,connecid=" << m_conn_index << ";" << send_buf.get_buffer_info());
				if (IsXMark(socketmk_isclose_sendfail))
				{
					//如果是客户端的链接
					if (IsXMark(socketmk_server_client))
					{
						vector<tcp_pak_header> vecmsgheads;
						send_buf.debug_print_buffermsglist(m_conn_index, vecmsgheads);
						WriteNetBufferMsgListInfo(m_conn_index, vecmsgheads);
					}
					NETLOG_ERROR("[net.HttpSession.send]关闭连接,缓冲区满,发送失败;connecid:" << m_conn_index << "; " << send_buf.get_buffer_info());
				}
				return 9070104;
			}
			return SubCheckSendMsgHead(send_buf);
		}

		int INiceNetSocket::SubCheckSendMsgHead(SendBufferType &send_buf)
		{
			unsigned uData_len = 0;
			void    *pSend = send_buf.front(uData_len);
			tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
			if (((unsigned)ppak_header->GetLen()) > send_buf.get_buffer_maxlen())
			{
				//我日，包的长度比整个缓冲区还大，很明显包头错误
				NETLOG_ERROR("[数据包头错误,将关闭连接]<665> 包头太长! pHeader->Len=" << ppak_header->GetLen() << ",conn_index=" << GetConnectIndex() << FUN_LINE);
				return 9070106;
			}

			if (ppak_header->GetLen() < sc_pak_header_len)
			{
				//我日，包的长度比包头还小，明显错误
				NETLOG_ERROR("[数据包头错误,将关闭连接]<8> 包头太小[<12]! pHeader->Len=" << ppak_header->GetLen() << LogSelf());
				return 9070107;
			}

			if (!ppak_header->check())
			{
				NETLOG_ERROR("[数据包头错误,将关闭连接]<10> pHeader->Len=" << ppak_header->GetLen() << LogSelf() << ";CheckCode=" << ppak_header->getCheckCode());
				return 9070108;
			}
			return 0;
		}

		void INiceNetSocket::SubFunBuildSelfLog()
		{
			unsigned uRport;
			string   strRomeIP = this->GetRemoteIp(uRport);
			unsigned uLocalport;
			string   strLocalIP = this->GetLocalIp(uLocalport);

			ostringstream strselflog;
			strselflog << " [Client:" << GetConnectIndex() << "] Local[" << strLocalIP << "," << uLocalport << "] Remote[" << strRomeIP << "," << uRport << "]";
			if(strselflog.str().size() < sizeof(m_selflog))
			{
				memset(m_selflog, 0, sizeof(m_selflog));
				strcpy(m_selflog, strselflog.str().c_str());
			}
			else
			{
				memset(m_selflog, 0, sizeof(m_selflog));
				strcpy(m_selflog, " TCPClientSession_handle_connect_zhtcc; ");
			}
		}

	}
}
