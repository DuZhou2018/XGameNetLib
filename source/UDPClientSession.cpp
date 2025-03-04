#include "../header/UDPClientSession.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CGlobal.hpp"

#ifdef WIN32
#pragma warning(disable:4503)
#endif

namespace peony {
	namespace net {

		UDPClientSession::UDPClientSession(unsigned conn_index, 
			TNewClientParam &param,
			TXZTBaseInitPm  &initPM,
			io_context      &io_service,
			AsioStrand	    &strand )
			:
			m_resolver(io_service),
			m_strand(strand),
			m_Param(param),
			m_socket(io_service),
			m_recv_buf(initPM.r_alloc, initPM.r_size),
			m_send_buf(initPM.s_alloc, initPM.s_size)
		{
			m_pStatData      = initPM.pStatData;
			m_conn_index     = conn_index;
			m_InFunConnect   = boost::bind(&UDPClientSession::ImpPackIAppClientOnConnect,this,_1,_2,_3,_4);
			m_runmsg_handler = boost::bind(&IAppClient::OnRecive,param.pApp,_1,_2,_3);
			this->SetXMark(socketmk_client|socketmk_log,0);
			m_SockType       = INiceNetSocket::ESockType_Client;
			strcpy(m_SocketTypeName, "UdpClient");
		}

		UDPClientSession::~UDPClientSession(void)
		{
			NETLOG_ERROR("[~UDPClientSession]" << this->LogSelf() << FUN_LINE);
            //CAutoLogDeadlock  AutoLgDL( " Connectid=",this->GetConnectID(),__FUNCTION__ );
			switch(m_EmConnectionState)
			{
			case CS_RESOLVING:
				post_connection_handler_zhtcc(GetConnectIndex(),CI_ADDR_RESOVLE_FAILED,"Unknown error.");
				break;
			case CS_CONNECTING:
				post_connection_handler_zhtcc(GetConnectIndex(),CI_CONNECTION_FAILED,"Unknown error.");
				break;
			default:
				break;
			}
		}

		void UDPClientSession::ImpPackIAppClientOnConnect(unsigned uMyServerID, unsigned uConnectID,int status, string info)
		{
			m_Param.pApp->OnConnect( uConnectID,status,info );
		}

		void UDPClientSession::StartSocket()
		{
            udp::resolver::query query(udp::v4(), m_Param.strServerIp, m_Param.strServerPort);
            if(1)
            {
                m_EmConnectionState = CS_RESOLVING;
                m_resolver.async_resolve(query,
                    m_strand.wrap(boost::bind(&INiceNetSocket::handle_udp_resolve_zhtcc, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::iterator)));
            }
            else
            {
                //udp::resolver resolver( PNGB::m_pScheduler->get_impl()->get_ioservice() );
                m_despoint = *m_resolver.resolve(query);

                m_IsConnectOk = true;
                this->SetXMark(socketmk_IsOpenState);
                m_LastLiveTm = PNGB::m_server_curTm;
                m_socket.open( udp::v4() );
                //启动接收消息的工作
                //XBase_ReadData();
            }
		}
        
		void UDPClientSession::handle_udp_resolve_zhtcc(const ZBoostErrCode& err,udp::resolver::iterator endpoint_iterator)
		{
			if( !err )
			{
                post_connection_handler_zhtcc( GetConnectIndex(),CI_ADDR_RESOVLE_SUCCESSED,"address resolved successfully" );		

				m_EmConnectionState = CS_CONNECTING;
				m_despoint = *endpoint_iterator;
                
				m_socket.async_connect(m_despoint,
					m_strand.wrap(boost::bind(&INiceNetSocket::handle_udp_connect_zhtcc, shared_from_this(),
						boost::asio::placeholders::error, ++endpoint_iterator)));
			}
			else
			{
				m_EmConnectionState = CS_NONE;
				post_connection_handler_zhtcc(GetConnectIndex(),CI_ADDR_RESOVLE_FAILED,"ZHT.resovle fail!");
                NETLOG_ERROR("connecid:"<<GetConnectIndex()<<";CI_ADDR_RESOVLE_FAILED; Err:"<<err.message()<<FUN_FILE_LINE);
			}
		}

		void UDPClientSession::handle_udp_connect_zhtcc(const ZBoostErrCode& err, udp::resolver::iterator endpoint_iterator)
		{
			if (!err)
			{
				// The connection was successful.
				m_EmConnectionState = CS_CONNECTED;
				post_connection_handler_zhtcc(GetConnectIndex(), CI_Connect_Success, "connection established");
				XBase_ReadData();

				//self log
				SubFunBuildSelfLog();
			}
			else if (endpoint_iterator != udp::resolver::iterator())
			{
				//继续链接下一个地址，一个域名可能有多个地址
				m_socket.close();
				m_despoint = *endpoint_iterator;

				m_socket.async_connect(m_despoint,
					m_strand.wrap(boost::bind(&INiceNetSocket::handle_udp_connect_zhtcc, shared_from_this(),
						boost::asio::placeholders::error, ++endpoint_iterator)));
			}
			else
			{
				NETLOG_ERROR("tcpclientsession connect fail! connecid:[" << GetConnectIndex() << "," << m_Param.strServerIp << "," << m_Param.strServerPort << "]; errorinfo:" << err.message() << FUN_LINE);
				m_EmConnectionState = CS_NONE;
				post_connection_handler_zhtcc(GetConnectIndex(), CI_CONNECTION_FAILED, "zht.connect fail!");
			}

		}

		void UDPClientSession::post_connection_handler_zhtcc(unsigned conn_index,EmConnectInfo info,const std::string  & msg)
		{
			mainrun_call_onconnected(0,conn_index,info,msg,shared_from_this() );
		}
		bool UDPClientSession::CheckHeartTiimeout()
		{
			return this->CheckHeartTimeout( m_Param.uHeartTmSecond );
		}
		bool UDPClientSession::Run(unsigned uCount)
		{
			/*
				这个函数是主线程调用的，使用的锁的顺序；
				nicenet模块的locker.
				消息来源的socket的recivelocker；
				消息来源socket的senderlocker/别的socket的senderlocker
			*/

			//CAutoLogDeadlock AutoLgDL(__FUNCTION__);
			Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);

			unsigned data_len = 0;
			void *pBuffer = m_recv_buf.front(data_len);
			if( (data_len>=sc_pak_header_len) && (data_len<=m_recv_buf.get_buffer_maxlen()) )
			{
				if( true )
				{
					if( data_len<g_MsgHeadLen )
					{
						NETLOG_ERROR("[消息处理.net] 消息长度异常 lostmsg,"<<LogSelf() );	
						return false;
					}
				}

				recvbuf_lock.unlock();

				if( 0 )//关闭异常
				{
					if( !DoNetInCmd(GetConnectIndex(),pBuffer,data_len) )
					{
						if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
							m_runmsg_handler(GetConnectIndex(),pBuffer,data_len);
					}

				}else
				{
					try{
						m_pStatData->RecLastSecFlow.uValue += data_len;
						m_pStatData->RecLastMinFlow.uValue += data_len;

						if( !DoNetInCmd(GetConnectIndex(),pBuffer,data_len) )
						{
							if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
								m_runmsg_handler(GetConnectIndex(),pBuffer,data_len);
							else
							{
								//NETLOG_NORMAL("[消息处理.net] lostmsg,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
							}
						}else
						{
							//NETLOG_FATAL("[网络逻辑消息.net] 逻辑消息,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
						}
					}catch(...){
                        string strExceptionInfo = CInPubFun::LogExceptionMsgBuf(pBuffer, 6699005);
						NETLOG_FATAL("[消息处理异常.net.udp],"<<LogSelf()<<strExceptionInfo<<"; data_len["<<data_len<<"]" );
						PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
					}
				}

				recvbuf_lock.lock();
				m_recv_buf.pop();

				if(m_recv_buf.m_IsStopRec )
				{
                    this->XBase_ReadData();
				}
			}else
			{
				NETLOG_FATAL("[消息大小异常，主动关闭连接,net],"<<LogSelf()<<FUN_FILE_LINE );
				this->CloseSocketOut("MsgLenError!");
				return false;
			}
			return true;
		}
		int  UDPClientSession::SendMsg(const void* data_ptr, unsigned data_size)
		{
			Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
			bool send_buf_is_empty = m_send_buf.IsEmpty();
			bool IsNeedCloseSocket = false;
			int iErrID = SubCheckUdpIsMaySendMsg( m_send_buf, data_ptr, data_size, IsNeedCloseSocket );
			if (iErrID > 0)
			{
				NETLOG_ERROR("[发送失败,需要关闭, net层]. wirte<1>!  iErrID=" << iErrID << LogSelf() << FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				if( IsNeedCloseSocket )
				{
					CloseSocketOut("SendMsgErrorXx!");
				}
				return iErrID;
			}
			//async. send loop has been started, just return
			if (!send_buf_is_empty)
				return 0;

			unsigned uData_len = 0;
			void *pSend = m_send_buf.front(uData_len);
			iErrID = this->XBase_WriteData( (const char*)pSend,uData_len );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[异常, net层]. wirte<1>! "<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("SendMsgException1!");
				return 9090106;
			}
			return 0;
		}

		int UDPClientSession::XBase_ReadData()
		{
			try
			{
				//m_send_endpoint
				char *pBuffer = (char*)m_recv_buf.open_for_push(1024);
				m_socket.async_receive_from(
					boost::asio::buffer(pBuffer, 1024),
					m_serverpoint,
					boost::bind(&INiceNetSocket::XBase_UdpReadCb,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			catch(...)
			{
				NETLOG_ERROR("[net.异常..1]" << LogSelf() << FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("ReadMsgBodyException!");
				return 99908801;
			}
			return 0;
		}
		void UDPClientSession::XBase_UdpReadCb(const ZBoostErrCode& error, std::size_t bytes_transferred)
		{
			if( !error )
			{
				ostringstream strLog;
				strLog << LogSelf() << "; m_serverpoint=" << m_serverpoint<<", errMsg="<<error.message();
				string strAaa = strLog.str();
				NETLOG_SYSINFO("[net.XZhtUdpBase.ReadCb]:[接收成功] " << strAaa << LogSelf());
				m_recv_buf.finish_push_Udp((unsigned)bytes_transferred);
                this->zht_CallMRecvHandler();

				XBase_ReadData();
			}
			else
			{
				NETLOG_ERROR("[net.XZhtUdpBase.handle_read]:[网络关闭,读错误发生] " << LogSelf());
				if( this->IsXMark(socketmk_log) )
				{
					string strerrormsg = error.message();
					ostringstream straaa;
					straaa << ", message = [" << error.value() << ";" << error.message() << "]";
					strerrormsg = straaa.str();
					NETLOG_ERROR(LogSelf() << ", message = [" << error.value() << ";" << error.message() << "]" << FUN_FILE_LINE);
				}

				//inclose("HanleReadFailAa");
				//ZBoostErrCode ec;
				//m_socket.close(ec);
			}
		}

		int UDPClientSession::XBase_WriteData(const char *pBuffer, unsigned uMaxLen)
		{
			try
			{
				m_socket.async_send_to(boost::asio::buffer(pBuffer, uMaxLen),
					m_despoint,
					boost::bind(&INiceNetSocket::XBase_UdpWriteCb,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			catch (...)
			{
				NETLOG_ERROR("[async_write异常]:" << LogSelf() << FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				return 99908802;
			}
			return 0;
		}

		void UDPClientSession::XBase_UdpWriteCb(const ZBoostErrCode& error, std::size_t bytes_transferred)
		{
			if(!error)
			{
				string strerrormsg = error.message();
				NETLOG_SYSINFO("[net.XZhtUdpBase.handle_sendcb]:[发送成功] strerrormsg="<< strerrormsg << LogSelf());
				Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
				m_send_buf.pop();

				//get next
				bool ishappend_error = false;
				tcp_pak_header *ppak_header = SendNextCheckSendMsgHead(m_send_buf, ishappend_error);
				if( 0 == ppak_header )
					return;
				unsigned uData_len = ppak_header->GetLen();
				if( ishappend_error )
				{
					//sendbuf_lock.unlock();
					//CloseSocketOut("HandleWriteLogicError!");
					//sendbuf_lock.lock();
					NETLOG_ERROR("[net.UDPClientSession.XBase_UdpWriteCb]:[逻辑错误]....."<<LogSelf() );
					return;
				}
				XBase_WriteData( (char*)ppak_header, uData_len );
			}
			else
			{
				NETLOG_ERROR("[net.XZhtUdpBase.handle_read]:[网络关闭,读错误发生] " << LogSelf());
				if (this->IsXMark(socketmk_log))
				{
					string strerrormsg = error.message();
					ostringstream straaa;
					straaa << ", message = [" << error.value() << ";" << error.message() << "]";
					strerrormsg = straaa.str();
					NETLOG_ERROR(LogSelf() << ", message = [" << error.value() << ";" << error.message() << "]" << FUN_FILE_LINE);
				}

				//inclose("HanleReadFailAa");
				//ZBoostErrCode ec;
				//m_socket.close(ec);
			}
		}

	}//end of namespace net
}//end of namespace peony
