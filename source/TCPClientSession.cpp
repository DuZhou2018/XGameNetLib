#include "../header/CGlobal.hpp"
#include "../header/Scheduler.hpp"
#include "../header/TCPClientSession.hpp"

#ifdef WIN32
#pragma warning(disable:4503)
#endif

namespace peony {
	namespace net {

		TCPClientSession::TCPClientSession(unsigned conn_index, 
			TNewClientParam &param,
			TXZTBaseInitPm &initPM,
			io_context &io_service,
			AsioStrand& strand )
			:
			XTcpSocketBase(io_service,initPM ),
			m_resolver(io_service),
			m_strand(strand),
			m_Param(param)
		{
			m_conn_index     = conn_index;
			m_InFunConnect   = boost::bind(&TCPClientSession::ImpPackIAppClientOnConnect,this,_1,_2,_3,_4);
			m_runmsg_handler = boost::bind(&IAppClient::OnRecive,param.pApp,_1,_2,_3);
			this->SetXMark(socketmk_client,0);
			m_SockType       = INiceNetSocket::ESockType_Client;
			strcpy(m_SocketTypeName, "TcpClient");
		}

		TCPClientSession::~TCPClientSession(void)
		{
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

		void TCPClientSession::ImpPackIAppClientOnConnect(unsigned uMyServerID, unsigned uConnectID,int status, string info)
		{
			m_Param.pApp->OnConnect( uConnectID,status,info );
		}

		bool TCPClientSession::XxTcpBeginRecWork()
		{
			read_packet_header();
			return true;
		}

		void TCPClientSession::StartSocket()
		{
			// Start an asynchronous resolve to translate the server and service names
			// into a list of endpoints.
			tcp::resolver::query query(m_Param.strServerIp,m_Param.strServerPort);

			//NETLOG_SYSINFO("[创建线程.一个奇怪的线程] threadid=0" );
			//下面的语句会创建一个线程，为什么，我还没有明白. 2017-03-25  zhanghongtao
			m_EmConnectionState = CS_RESOLVING;
 			m_resolver.async_resolve(query,
 				m_strand.wrap(boost::bind(&INiceNetSocket::handle_resolve_zhtcc,shared_from_this(),
 				boost::asio::placeholders::error,
 				boost::asio::placeholders::iterator)));
		}

		void TCPClientSession::handle_resolve_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator)
		{
			if( !err )
			{
                post_connection_handler_zhtcc( GetConnectIndex(),CI_ADDR_RESOVLE_SUCCESSED,"address resolved successfully" );

				// Attempt a connection to the first endpoint in the list. Each endpoint
				// will be tried until we successfully establish a connection.
				m_EmConnectionState = CS_CONNECTING;
				tcp::endpoint endpoint = *endpoint_iterator;

 				GetTcpSocket()->async_connect(endpoint,
 					m_strand.wrap(boost::bind(&INiceNetSocket::handle_connect_zhtcc,shared_from_this(),
 					boost::asio::placeholders::error, ++endpoint_iterator)));				
			}
			else
			{
				m_EmConnectionState = CS_NONE;
				post_connection_handler_zhtcc(GetConnectIndex(),CI_ADDR_RESOVLE_FAILED,"ZHT.resovle fail!");
                NETLOG_ERROR("connecid:"<<GetConnectIndex()<<";CI_ADDR_RESOVLE_FAILED; Err:"<<err.message()<<FUN_FILE_LINE);
			}
		}
		void TCPClientSession::handle_connect_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator)
		{
			if( !err )
			{
				// The connection was successful.
				m_EmConnectionState = CS_CONNECTED;
                post_connection_handler_zhtcc( GetConnectIndex(),CI_Connect_Success,"connection established" );
				OpenState();

				//self log
				SubFunBuildSelfLog();
			}
            else if( endpoint_iterator != tcp::resolver::iterator() )
            {
                NETLOG_ERROR(__FUNCTION__<<" connecid:"<<GetConnectIndex()<<";the endpoint fail;");
                // The connection failed. Try the next endpoint in the list.
				GetTcpSocket()->close();
                tcp::endpoint endpoint = *endpoint_iterator;

				GetTcpSocket()->async_connect(endpoint,
                    m_strand.wrap(boost::bind(&INiceNetSocket::handle_connect_zhtcc,shared_from_this(),
                    boost::asio::placeholders::error, ++endpoint_iterator)));
            }
            else
            {
				NETLOG_ERROR("tcpclientsession connect fail! connecid:["<<GetConnectIndex()<<","<<m_Param.strServerIp<<","<<m_Param.strServerPort<<"]; errorinfo:"<<err.message()<<FUN_LINE );
                m_EmConnectionState = CS_NONE;
                post_connection_handler_zhtcc(GetConnectIndex(),CI_CONNECTION_FAILED,"zht.connect fail!");
            }
		}
		void TCPClientSession::post_connection_handler_zhtcc(unsigned conn_index,EmConnectInfo info,const std::string  & msg)
		{
			mainrun_call_onconnected(0,conn_index,info,msg,shared_from_this() );
			//PNGB::m_pScheduler->get_impl()->post(boost::bind(m_conn_handler,conn_index,info,msg,shared_from_this()) );
		}

		bool TCPClientSession::CheckHeartTiimeout()
		{
			return this->CheckHeartTimeout( m_Param.uHeartTmSecond );
		}
		bool TCPClientSession::Run(unsigned uCount)
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
					{//g_MsgHeadLen
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
                        string strExceptionInfo = CInPubFun::LogExceptionMsgBuf( pBuffer,6699001 );
						NETLOG_FATAL("[消息处理异常.net.tcp1],"<<LogSelf()<<strExceptionInfo<<"; data_len["<<data_len<<"]" );
						PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
					}
				}

				recvbuf_lock.lock();
				m_recv_buf.pop();

				if(m_recv_buf.m_IsStopRec )
				{
					this->Tcp_read_packet_body();
				}
			}else
			{
				NETLOG_FATAL("[消息大小异常，主动关闭连接,net],"<<LogSelf()<<FUN_FILE_LINE );
				this->CloseSocketOut("MsgLenError!");
				return false;
			}
			return true;
		}
		int  TCPClientSession::SendMsg(const void* data_ptr, unsigned data_size)
		{
			Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
			bool send_buf_is_empty = m_send_buf.IsEmpty();
			bool IsNeedCloseSocket = false;
			int iErrID = SubCheckTcpIsMaySendMsg( m_send_buf, data_ptr, data_size, IsNeedCloseSocket);
			if( iErrID>0 )
			{
				NETLOG_ERROR("[发送失败,需要关闭, net层]. wirte<1>!  iErrID=" << iErrID << LogSelf() << FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				if (IsNeedCloseSocket)
				{
					CloseSocketOut("SendMsgErrorXx!");
				}
				return iErrID;
			}
			//async. send loop has been started, just return
			if( !send_buf_is_empty )
				return 0;
			
			unsigned uData_len = 0;
			void *pSend = m_send_buf.front(uData_len);
			iErrID = this->XBase_WriteData( (const char*)pSend,uData_len,uData_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[异常, net层]. wirte<1>! "<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);

				CloseSocketOut("SendMsgException1!");
				return 9090106;
			}
			return 0;
		}

		void TCPClientSession::read_packet_header()
		{
			m_recv_buf.m_pak_header_readed = false;
			char *pTtBuf = (char*)(&m_recv_buf.m_RHeader);
			int iErrID = this->XBase_ReadData( pTtBuf,sc_pak_header_len,sc_pak_header_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[net.异常..2]"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				inclose("ReadPackException");
				return;
			}
		}
		void TCPClientSession::XBase_ReadCb(const ZBoostErrCode& error,size_t bytes_transferred,int iDataType)
		{
			if(error)
			{
				NETLOG_ERROR("[net.TCPClientSession.handle_read]:[网络关闭,读错误发生] "<<LogSelf() );
				if( this->IsXMark(socketmk_log) )
				{
					string strerrormsg = error.message();
					NETLOG_ERROR(LogSelf()<<", message = ["<<error.value()<<";"<<error.message()<<"]"<<FUN_FILE_LINE );
				}

				inclose("HanleReadFailAa");
				ZBoostErrCode ec;
				m_socket.close(ec);
				return;
			}

			m_LastLiveTm = PNGB::m_server_curTm;
			int iresult = Tcp_in_do_handle_read(bytes_transferred);
			if( -1 ==  iresult )
			{
				inclose("LogicReadDataFail!");
				return;
			}else if( 1 == iresult )
			{//开始读包的数据信息
			}
			else if( 2 == iresult )
			{//读完了一个数据包，开始下一个数据包的读取
				//加入到消息队列中。
				this->zht_CallMRecvHandler();
				//开始读下一包数据
				read_packet_header();
			}
		}
		void TCPClientSession::XBase_WriteCb(const ZBoostErrCode& error,int iDataType)
		{
			if( !error )
			{
				Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
				// pop one pak has been sent
				m_send_buf.pop();

				// get next
				bool ishappend_error = false;
				tcp_pak_header *ppak_header = SendNextCheckSendMsgHead(m_send_buf, ishappend_error);
				if (0 == ppak_header)
					return;
				unsigned uData_len = ppak_header->GetLen();
				if( ishappend_error )
				{
					CloseSocketOut("HandleWriteLogicError!");
					return;
				}

				int iErrID = this->XBase_WriteData( (const char*)ppak_header, uData_len, uData_len,0 );
				if( iErrID>0 )
				{
					NETLOG_ERROR("[async_write异常]:"<<LogSelf()<<FUN_FILE_LINE);
					PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);

					CloseSocketOut("SendMsgException!");
					return;	
				}
				m_LastLiveTm = PNGB::m_server_curTm;
				if( PNGB::m_pLog->IsMayLog(Log_DebugNet) )
				{
					if( this->IsXMark(socketmk_server_client) && this->IsXMark(socketmk_log) )
					{
						NETLOG_DBNET("[SendPack]; ConID="<<GetConnectIndex()<<";SendPack["<<ppak_header->GetMsgID()<<";"<<ppak_header->GetLen()<<"]");
					}
				}
			}
			else
			{
				if( this->IsXMark(socketmk_log) )
				{
					NETLOG_ERROR(LogSelf()<<", error="<<error.message()<<",errorid="<<error.value()<<FUN_FILE_LINE );
				}

				CloseSocketOut("HandleWriteErrorAcc!");
				return;
			}
		}
	}	// end of namespace net

}	// end of namespace peony
