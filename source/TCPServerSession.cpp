

#include "../header/TCPServerSession.hpp"
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
        //	Implemention of TCPServerSession
        //

        TCPServerSession::~TCPServerSession()
        {
			//NETLOG_ERROR(this->LogSelf()<<FUN_FILE_LINE);
        }

		bool TCPServerSession::XxTcpBeginRecWork()
		{
			this->read_packet_header();
			return true;
		}

		TCPServerSession::TCPServerSession(io_context &io_service, 
			TXZTBaseInitPm &initPM, 
			const TNewServerParam &param)
			:
    		XTcpSocketBase(io_service,initPM ),
			m_param(param)
		{
			//CAutoLogDeadlock  AutoLgDL( " Connectid=",__FUNCTION__ );
			this->SetXMark(socketmk_server_client,0);
			m_SockType  = INiceNetSocket::ESockType_Server_Tcp;
			strcpy(m_SocketTypeName, "Tcp");
		}
		void TCPServerSession::StartSocket()
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
					strcpy(m_selflog," TCPServerSession->start; " );
				}

			}
        }


		bool TCPServerSession::DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len)
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
            case INPROTOCOL::NiceNetP_Counter_CTOS_ReqList:
                {
                    //assert(sizeof(INPROTOCOL::TClientReqCounterList)==data_len);
					if( sizeof(INPROTOCOL::TClientReqCounterList)!=data_len )
					{
						NETLOG_ERROR(FUN_FILE_LINE);
						return true;
					}

                    EmIDKind IDKind;
                    unsigned uServer_instance_id;
                    if( PNGB::m_pCSIDMG->GetIndexIDByFactID(connectid,IDKind,uServer_instance_id) )
                    {
                        INPROTOCOL::LIST_RepCounterList listMsg;
                        PNGB::m_pCountersMg->GetCountersInfoListRegToServer(uServer_instance_id,listMsg);
						INPROTOCOL::LIST_RepCounterList::iterator it=listMsg.begin();
                        for(; it!=listMsg.end(); ++it )
                        {
                            INPROTOCOL::TServerRepCounterList &cmd = *it;

                            unsigned iNoUseLen = sizeof(INPROTOCOL::TServerRepCounterList::COUNTER_ITEM)*(INPROTOCOL::COUNTERLIST_MAXCOUNT-cmd.count);
							cmd.SetLen( sizeof(cmd)-iNoUseLen );
                            int bRet = this->SendMsg(&cmd,cmd.GetLen() );
                            if( bRet )
							{
								NETLOG_ERROR("���ͼ��������ʧ��,�п��ܷ��ͻ�������! bRet="<<bRet<<FUN_FILE_LINE);
							}
                        }
                    }

					//֪ͨ�߼��㣬������ӵ�����
					mainrun_call_onconnected(m_uXServerID,this->GetConnectIndex(),XS_MeIsOpcTool,"me is opctool!",shared_from_this() );
                    return true;
                }
            case INPROTOCOL::NiceNetP_Counter_CTOS_ReqBook:
                {
                    //assert(sizeof(INPROTOCOL::TClientReqBook)==data_len);
					if( sizeof(INPROTOCOL::TClientReqBook)!=data_len )
					{
						NETLOG_ERROR("[net�߼�����!]"<<FUN_FILE_LINE);
						return true;
					}

                    INPROTOCOL::TClientReqBook *pReq = (INPROTOCOL::TClientReqBook*)pdata;
                    INPROTOCOL::TServerRepBook repMsg;
                    repMsg.counter_id = pReq->counter_id;
                    repMsg.is_success = pReq->interval;
                    if( !PNGB::m_pCountersMg->InstallAndUnInstallCounter(connectid,pReq->counter_id,pReq->interval) )
                        repMsg.is_success = -1;
					//repMsg.Len = sizeof(repMsg);
					repMsg.SetLen( sizeof(repMsg) );
                    this->SendMsg(&repMsg,sizeof(repMsg));
                    return true;
                }
			case INPROTOCOL::NiceNetP_ZhtControl_LogicReq:
				{
					//assert(sizeof(INPROTOCOL::TClientReqBook)==data_len);
					if( sizeof(INPROTOCOL::NiceNetPZhtControlLogicReq)!=data_len )
					{
						NETLOG_ERROR("[net�߼�����!]"<<FUN_FILE_LINE);
						return true;
					}
					INPROTOCOL::NiceNetPZhtControlLogicReq *pReq = (INPROTOCOL::NiceNetPZhtControlLogicReq*)pdata;
					if( INPROTOCOL::NiceNetPZhtControlLogicReq::sczhtcontrlmarked != pReq->marked )
					{
						return true;
					}
					PNGB::NetMsg_ZhtControler( connectid,pReq );
					return true;
				}
			case INPROTOCOL::NiceNetP_NetLog_OpReader_Req:
				{
					if( sizeof(INPROTOCOL::TNiceNetPNetLogOpReaderReq)!=data_len )
					{
						NETLOG_ERROR("[net�߼�����!]"<<FUN_FILE_LINE);
						return true;
					}
					INPROTOCOL::TNiceNetPNetLogOpReaderReq *pReq = (INPROTOCOL::TNiceNetPNetLogOpReaderReq*)pdata;
					PNGB::m_pNetLogOpcTool->AddReader( connectid,pReq );
					return true;
				}
			case INPROTOCOL::NiceNetP_ZhtControl_CloseMeNetHeart:
				{//
					INPROTOCOL::NiceNetPZhtControlCloseMeNetHeartReq *pReq = (INPROTOCOL::NiceNetPZhtControlCloseMeNetHeartReq*)pdata;
					if( pReq->IsOpen )
						this->ClearXMark( socketmk_NoCheckHeart );
					else
						this->SetXMark( socketmk_NoCheckHeart );
					return true;
				}
            default:
                break;
            }
            return false;
        }

		int TCPServerSession::SendMsg(const void* data_ptr, unsigned data_size)
		{
			Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
			bool send_buf_is_empty = m_send_buf.IsEmpty();
			bool IsNeedCloseSocket = false;
			int iErrID = SubCheckTcpIsMaySendMsg(m_send_buf, data_ptr, data_size, IsNeedCloseSocket);
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
			//async. send loop has been started, just return
			if (!send_buf_is_empty)
				return 0;

			unsigned uData_len = 0;
			void *pSend = m_send_buf.front(uData_len);
			iErrID = this->XBase_WriteData( (const char*)pSend,uData_len,uData_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[�쳣, net��]. wirte<1>! "<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);

				CloseSocketOut("SendMsgException1!");
				return 9090106;
			}
			return 0;
		}

		bool TCPServerSession::Run(unsigned uCount)
		{
			/*
				������������̵߳��õģ�ʹ�õ�����˳��
				nicenetģ���locker.
				��Ϣ��Դ��socket��recivelocker��
				��Ϣ��Դsocket��senderlocker/���socket��senderlocker
			*/

			//CAutoLogDeadlock AutoLgDL(__FUNCTION__);
			Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
			unsigned data_len = 0;
			void    *pBuffer  = m_recv_buf.front(data_len);

			if( (data_len>=sc_pak_header_len) && (data_len<=m_recv_buf.get_buffer_maxlen()) )
			{
				if( true )
				{
					if( data_len<g_MsgHeadLen )
					{
						NETLOG_ERROR("[��Ϣ����.net] ��Ϣ�����쳣 lostmsg,"<<LogSelf() );	
						return false;
					}
				}

				recvbuf_lock.unlock();

				if( 0 )//�ر��쳣
				{
					if( !DoNetInCmd(GetConnectIndex(),pBuffer,data_len) )
					{
						if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
						{
							m_param.pApp->OnRecive(GetConnectIndex(),pBuffer,data_len);
						}
					}
				}else
				{
					try{
						m_pStatData->RecLastSecFlow.uValue += data_len;
						m_pStatData->RecLastMinFlow.uValue += data_len;

						if( !DoNetInCmd(GetConnectIndex(),pBuffer,data_len) )
						{
							if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
							{
								m_param.pApp->OnRecive(GetConnectIndex(),pBuffer,data_len);
							}else
							{
								//NETLOG_NORMAL("[��Ϣ����.net] lostmsg,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
							}
						}
					}catch(...){
                        string strExceptionInfo = CInPubFun::LogExceptionMsgBuf(pBuffer, 6119003);
                        NETLOG_FATAL("[��Ϣ�����쳣.net.tcp],"<<LogSelf()<<strExceptionInfo<<"; data_len["<<data_len<<"]");
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
				NETLOG_FATAL("[��Ϣ��С�쳣�������ر�����,net],"<<LogSelf()<<FUN_FILE_LINE );
				CloseSocketOut("MsgLenError!");
				return false;
			}
			return true;
		}

		void TCPServerSession::read_packet_header()
		{
			m_recv_buf.m_pak_header_readed = false;
			char *pTtBuf = (char*)(&m_recv_buf.m_RHeader);
			int iErrID = this->XBase_ReadData( pTtBuf,sc_pak_header_len,sc_pak_header_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[net.�쳣..2]"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				inclose("ReadPackException");
				return;
			}
		}
		void TCPServerSession::XBase_ReadCb(const ZBoostErrCode& error,size_t bytes_transferred,int iDataType)
		{
			if(error)
			{
				NETLOG_SYSINFO("[net.TCPServerSession.handle_read]:[����ر�,��������] "<<LogSelf()<<FUN_LINE );
				if( this->IsXMark(socketmk_log) )
				{
					string strerrormsg = error.message();
					NETLOG_ERROR(LogSelf()<<", message = ["<<error.value()<<";"<<error.message()<<"]"<<FUN_FILE_LINE );
				}

				inclose("HanleReadFailBb");
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
			{//��ʼ������������Ϣ
			}
			else if( 2 == iresult )
			{//������һ�����ݰ�����ʼ��һ�����ݰ��Ķ�ȡ
				//���뵽��Ϣ�����С�
				this->zht_CallMRecvHandler();
				//��ʼ����һ������
				read_packet_header();
			}
		}
		void TCPServerSession::XBase_WriteCb(const ZBoostErrCode& error,int iDataType)
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
					return;

				bool ishappend_error		= false;
				tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
				if( ((unsigned)ppak_header->GetLen()) > m_send_buf.get_buffer_maxlen())
				{
					//���գ����ĳ��ȱ��������������󣬺����԰�ͷ����
					ishappend_error = true;
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] ��ͷ̫��! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_FILE_LINE);
				}

				if( ppak_header->GetLen()<sc_pak_header_len )
				{
					//���գ����ĳ��ȱȰ�ͷ��С�����Դ���
					ishappend_error = true;
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] ��ͷ̫С[<12]! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_FILE_LINE);
				}

				if( !ppak_header->check() )
				{
					NETLOG_ERROR("[���ݰ�ͷ����,���ر�����] pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<";CheckCode="<<ppak_header->getCheckCode()<<FUN_FILE_LINE);
					ishappend_error = true;
				}

				if( ishappend_error )
				{
					CloseSocketOut("HandleWriteLogicError!");
					return;
				}

				int iErrID = this->XBase_WriteData( (const char*)pSend,uData_len,uData_len,0 );
				if( iErrID>0 )
				{
					NETLOG_ERROR("[async_write�쳣]:"<<LogSelf()<<FUN_FILE_LINE);
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

				CloseSocketOut("HandleWriteErrorAaee!");
				return;
			}
		}


	}	// end of namespace net

}	// end of namespace peony
