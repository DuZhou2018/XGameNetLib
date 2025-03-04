#include "../sha1/sha1.h"
#include "../include/NiceNetBase64.h"
#include "../header/NiceNet.h"
#include "../header/NetLogMg.h"
#include "../header/CGlobal.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CountersMg.hpp"
#include "../header/WSSocketBase.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/InPublicPreDefine.hpp"
#include "../json/json.h"

namespace peony
{
	namespace net 
	{
		CWSSocketBase::CWSSocketBase(io_context &io_service, TXZTBaseInitPm &initPM)
		:XTcpSocketBase(io_service,initPM )
		{
			this->SetXMark( socketmk_log );
			memset(m_ConnectBuf,0,sizeof(m_ConnectBuf) );

			m_SockType = INiceNetSocket::ESockType_Server_Wss;
			strcpy(m_SocketTypeName, "WebSocket");
			m_MsgHead.Reset();
		}

		CWSSocketBase::~CWSSocketBase()
		{
			if( IsXMark(socketmk_log) )
			{
				NETLOG_DBNET("destory HttpSession, Connectid="<<m_conn_index);
			}

			//20161017,zht add this line code
			m_socket.close();
			this->ClearAllXMark();
		}
		bool CWSSocketBase::XxTcpBeginRecWork()
		{
			return true;
		}

		void CWSSocketBase::XBase_ReadCb(const ZBoostErrCode& error,size_t bytes_transferred,int iDataType)
		{
			XHandReadType xReadType = (XHandReadType)iDataType;
			//<1>�ж��Ƿ����˴���
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
			switch (xReadType)
			{
			case EHReadType_LogicConnect:
				{
					HandRead_LogicConnect( bytes_transferred );
					break;
				}
				//�����������һֱѭ����
			case EHReadType_LogicMsgHeadFirst:
				{
					HandRead_LogicMsgHeadFirst(bytes_transferred );
					break;
				}
			case EHReadType_LogicMsgHeadSecond:
				{
					HandRead_LogicMsgHeadSecond(bytes_transferred );
					break;
				}
			case EHReadType_LogicMsgBody:
				{
					HandRead_LogicMsgBody(bytes_transferred );
					break;
				}
			default:
				{
					NETLOG_ERROR(FUN_FILE_LINE);
					break;
				}
			}
			//��ʼһ�ζ�����
			this->BeginOnceReadLogicMsg();
		}

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		bool CWSSocketBase::BeginOnceReadLogicMsg()
		{
			//�жϰ�ͷ�Ƿ����ɣ�
			if( false == m_MsgHead.HeadRecOver )
			{
				if( false == m_MsgHead.HearFirstIsRead )
				{//�жϰ�ͷ��ǰ�����ֽ��Ƿ��ȡ
					m_MsgHead.HearFirstIsRead = true;
					ImpRecMaxLenMsg( EHReadType_LogicMsgHeadFirst, m_MsgHead.HeadBuffer,2,2 );
				}else
				{//����ͷʣ�µĲ���
					if( m_MsgHead.HeadSecondPartLen<1 || m_MsgHead.HeadSecondPartLen>12 )
					{
						this->CloseSocketOut("WebCalHeadSecondLen Error...");
						return false;
					}

					//��Ƕ���ͷ��ɵı��
					m_MsgHead.HeadRecOver = true;
					//����ͷ�ĵڶ�����
					ImpRecMaxLenMsg( EHReadType_LogicMsgHeadSecond,m_MsgHead.HeadBuffer+2,m_MsgHead.HeadSecondPartLen,m_MsgHead.HeadSecondPartLen );
				}
			}else
			{//������
				ImpRecMaxLenMsg_ReadBody();
			}
			return true;
		}
		int  CWSSocketBase::ImpRecMaxLenMsg_ReadBody()
		{
			//if( m_MsgHead.uMsgLength<1 )
			//	return 93020601;

			RecvBufferType *recv_buf = &m_recv_buf;
			void *pBuffer = recv_buf->open_for_push( m_MsgHead.uMsgLength+sc_pak_header_len );
			if( 0== pBuffer )
			{
				recv_buf->m_IsStopRec = true;
				return 93020602;
			}

			//����ϵͳ����
			m_recv_buf.m_IsStopRec = false;

			//������ʱ��ͷ
			tcp_pak_header &tMsgHead = recv_buf->m_RHeader;
			tMsgHead.reset();
			tMsgHead.SetLen( m_MsgHead.uMsgLength+sc_pak_header_len );
			tMsgHead.SetWebosockDataType( m_MsgHead.iMsgType );

			//������ͷ
			memcpy(pBuffer, &tMsgHead, sc_pak_header_len );

			if( 0==m_MsgHead.uMsgLength )
			{//������ݰ�û�а���
				HandRead_LogicMsgBody(0);
			}else
			{
				m_MsgHead.pBodyPosInRecBuf = (char*)pBuffer+sc_pak_header_len;
				ImpRecMaxLenMsg( EHReadType_LogicMsgBody,m_MsgHead.pBodyPosInRecBuf,m_MsgHead.uMsgLength,m_MsgHead.uMsgLength );
			}
			return 0;
		}
		void CWSSocketBase::HandRead_LogicMsgHeadFirst( size_t bytes_transferred )
		{
			//Opcode
			// 	���ݰ����ͣ�frame type����ռ4bits
			// 	0x0����ʶһ���м����ݰ�
			// 	0x1����ʶһ��text�������ݰ�
			// 	0x2����ʶһ��binary�������ݰ�
			// 	0x3-7������
			// 	0x8����ʶһ���Ͽ������������ݰ�
			// 	0x9����ʶһ��ping�������ݰ�
			// 	0xA����ʾһ��pong�������ݰ�
			// 	0xB-F������

			unsigned char chFirstByte = (unsigned char)m_MsgHead.HeadBuffer[0];
			//���λ����������Ϣ�Ƿ����,���Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�
			m_MsgHead.IsMsgEof = !!(chFirstByte>>7);
			//�ж���Ϣ����
			m_MsgHead.iMsgType = chFirstByte&(0XF);
			unsigned char chSecondByte = (unsigned char)m_MsgHead.HeadBuffer[1];
			//�ж��Ƿ������봦��
			m_MsgHead.IsHaseMask = !!(chSecondByte>>7);
			if( m_MsgHead.IsHaseMask )
				m_MsgHead.HeadSecondPartLen = 4;

			if( 0X8==m_MsgHead.iMsgType )
			{
				int iXss = 0;
				iXss++;
            }
            else if(0X9==m_MsgHead.iMsgType)
            {//��ʶһ��ping�������ݰ�
                int iXss = 0; iXss++;
            }
			//NETLOG_SYSINFO("Wss, ������...iMsgType="<<m_MsgHead.iMsgType<<FUN_FILE_LINE);

			//��Ϣ�ĳ���
			m_MsgHead.uMsgLength = (unsigned)(chSecondByte & 0x7F);
			if( 0==m_MsgHead.uMsgLength )
			{//ֻ�а�ͷ��û����Ϣ����,�����Ϣ�����
				//��ͷ������ˣ�
				m_MsgHead.HeadRecOver = true;
			}
			else if( 126>m_MsgHead.uMsgLength )
			{
				/*
				memcpy( m_MsgHead.chMask,&m_ConnectBuf[2],sizeof(m_MsgHead.chMask) );
				unsigned char tempCaa[256]={0};
				for( unsigned i = 0; i < m_MsgHead.uMsgLength; i++)
				{
				tempCaa[i] = (m_ConnectBuf[6+i] ^ m_MsgHead.chMask[i % 4]);
				}
				*/
			}else if( 126==m_MsgHead.uMsgLength )
			{
				m_MsgHead.HeadSecondPartLen += 2;

				/*
				m_MsgHead.uMsgLength = ntohs( *(unsigned short*) (m_ConnectBuf+2) );
				memcpy( m_MsgHead.chMask,&m_ConnectBuf[4],sizeof(m_MsgHead.chMask) );

				unsigned char tempCaa[1024]={0};
				for( unsigned i = 0; i < m_MsgHead.uMsgLength; i++)
				{
				tempCaa[i] = (m_ConnectBuf[8+i] ^ m_MsgHead.chMask[i % 4]);
				}
				*/
			}else
			{
				this->CloseSocketOut("WebMsgTooBigAcca");
			}

			if( 0==m_MsgHead.HeadSecondPartLen )
			{//������İ�ͷ��2���ֽڣ�����û������
				m_MsgHead.HeadRecOver = true;
				if( 0==m_MsgHead.uMsgLength )
				{
					//���������ĳ���Ϊ0����ֻ�а�ͷ�������ֽ�
					//����������ɣ�����û��ʲô����
					NETLOG_ERROR("[�յ���һ���հ�]"<<FUN_FILE_LINE );
					m_MsgHead.Reset();
				}else
				{
					//����һ���ְ��壬��ô�ͽ��հ����
				}
			}else
			{
				//û��ʲôҪ����ģ��������հ����
			}
		}
		void CWSSocketBase::HandRead_LogicMsgHeadSecond( size_t bytes_transferred )
		{
			
			if( 126>m_MsgHead.uMsgLength )
			{
				//��ȡ����
				if( m_MsgHead.IsHaseMask )
					memcpy( m_MsgHead.chMask,&m_MsgHead.HeadBuffer[2],sizeof(m_MsgHead.chMask) );
			}else if( 126==m_MsgHead.uMsgLength )
			{
				//��ȡ��Ϣ�������ʵ����
				m_MsgHead.uMsgLength = ntohs( *(unsigned short*) (m_MsgHead.HeadBuffer+2) );
				if( m_MsgHead.IsHaseMask )
					memcpy( m_MsgHead.chMask,&m_MsgHead.HeadBuffer[4],sizeof(m_MsgHead.chMask) );
			}else
			{
				NETLOG_ERROR(FUN_FILE_LINE);
			}

			if( 0==m_MsgHead.uMsgLength )
			{
				//���������ĳ���Ϊ0����ֻ�а�ͷ�������ֽ�
				//����������ɣ�����û��ʲô����
				NETLOG_ERROR("[�յ���һ���հ�aa]"<<FUN_FILE_LINE );
				m_MsgHead.Reset();
			}

			//NETLOG_SYSINFO("[ �յ�WebSocket��Ϣ <<-- ] : "<<m_MsgHead.LogInfo()<<FUN_FILE_LINE );
		}
		void CWSSocketBase::HandRead_LogicMsgBody( size_t bytes_transferred )
		{
			if( bytes_transferred != m_MsgHead.uMsgLength )
			{
				this->CloseSocketOut("WebSockLogicErradea!");
				NETLOG_ERROR("m_MsgHead.uMsgLength="<<m_MsgHead.uMsgLength<<"; bytes_transferred="<<bytes_transferred<<FUN_FILE_LINE);
				return;
			}
			//unsigned char*pDebugSSs = (unsigned char*)(m_MsgHead.pBodyPosInRecBuf);
			if( m_MsgHead.IsHaseMask )
			{
				//���н���
				unsigned char uTempChAaa = 0;
				unsigned char*pCurChar   = (unsigned char*)(m_MsgHead.pBodyPosInRecBuf);
				for( unsigned tIndex = 0; tIndex<m_MsgHead.uMsgLength; tIndex++ )
				{
					uTempChAaa = *pCurChar;
					*pCurChar   = (uTempChAaa^m_MsgHead.chMask[tIndex%4]);
					pCurChar  += 1;
				}
			}

			if( !m_recv_buf.finish_push((unsigned)(bytes_transferred+sc_pak_header_len) ) )
			{
				NETLOG_ERROR("connecid:"<<m_conn_index<<";finish_push() fail!"<<FUN_FILE_LINE);
				this->CloseSocketOut("WebSssLogicErracRd");
				return;
			}
			this->zht_CallMRecvHandler();
			m_MsgHead.Reset();
		}
		int  CWSSocketBase::ImpRecMaxLenMsg( XHandReadType xReadType, char *pRecBuffer,unsigned uMaxLen, unsigned uMinLen)
		{
			int iErrID = this->XBase_ReadData( pRecBuffer,uMaxLen,uMinLen,xReadType );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[net.�쳣..1]"<<LogSelf()<<FUN_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("ReadMsgBodyException!");
				return 1250102;
			}
			return 0;
		}
		
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//pushһ����Ϣ�����ͻ�����
		int  CWSSocketBase::SendMsg(const char *pdata,unsigned data_len)
		{
			return this->SendMsg_Aaa( pdata,data_len,0X81 );
		}

		int CWSSocketBase::SendWebSocketMsg(const void *pdata,unsigned data_len,bool IsText)
		{
            int  iDataType = IsText?0X81:0X82;
			return this->SendMsg_Aaa((const char*)pdata,data_len,iDataType );
		}

		//pushһ����Ϣ�����ͻ�����
        //iDataType( 0X81[һ���ı����ݰ�]; 0X82[һ�����������ݰ�]; 0X8A(һ��pong���ݰ�) )
		int  CWSSocketBase::SendMsg_Aaa( const char *pdata,unsigned data_len, int iDataType )
		{
			static unsigned sc_msgMax = 102400;
			static unsigned char *g_WebSendBuf = 0;
			if( 0==g_WebSendBuf )
			{
				g_WebSendBuf = new unsigned char[sc_msgMax];
			}
			if( data_len+100>sc_msgMax )
				return 9320302;

			memset(g_WebSendBuf,0,sc_msgMax);

			//���ɷ��ͻ������İ�ͷ
			int  iCurBufPos = sc_pak_header_len;
			tcp_pak_header *pMsgHead = (tcp_pak_header*)g_WebSendBuf;

			//д��ͷ�ĵ�һ���̶��ֽ�
			//unsigned char chFirstByte = 0X81(�ı�) 0X82(������);
			//unsigned char chFirstByte = IsText?0X81:0X82;
            unsigned char chFirstByte = iDataType;
            g_WebSendBuf[iCurBufPos] = chFirstByte;
			iCurBufPos += 1;

			//д��ͷ�ĳ��ȵڶ����ֽ�
			unsigned short uTureMsgLen = data_len;
			if( data_len<126 )
			{
				g_WebSendBuf[iCurBufPos] = (unsigned char)data_len;
				iCurBufPos += 1;
			}else if( data_len<65000 )
			{
				g_WebSendBuf[iCurBufPos] = 126;
				iCurBufPos += 1;
				//д����
				uTureMsgLen = ntohs(uTureMsgLen);
				memcpy( &g_WebSendBuf[iCurBufPos],&uTureMsgLen,sizeof(uTureMsgLen) );
				iCurBufPos += 2;
			}

			if( 0 /*uMask>0*/ )
			{
				unsigned uMask = 34242;
				//д����
				memcpy( &g_WebSendBuf[iCurBufPos],&uMask,sizeof(uMask) );
				iCurBufPos += sizeof(uMask);

				//��������
				unsigned char *pSourData = (unsigned char*)pdata;
				unsigned char chTempMask[10]={0};
				memcpy(chTempMask,&uMask,sizeof(uMask) );
				for( unsigned tIndex=0;tIndex<data_len; tIndex++ )
				{
					unsigned char tCurChar   = *pSourData;
					g_WebSendBuf[iCurBufPos] =  tCurChar^chTempMask[tIndex%4];
					iCurBufPos += 1;
					pSourData  += 1;
				}
			}else
			{
				//д����
				memcpy( &g_WebSendBuf[iCurBufPos],pdata, data_len );
				iCurBufPos += data_len;
			}
			pMsgHead->SetLen( iCurBufPos );
			pMsgHead->SetCheckCode();
			this->PushSendQueue( pMsgHead,pMsgHead->GetLen() );
			return 0;
		}

        int  CWSSocketBase::PushSendQueue(const void* data_ptr, size_t data_size)
		{
			Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
			if( false == this->IsGoodConnect() )
				return 9090101;

			if( !m_socket.is_open() )
			{
				NETLOG_ERROR("[net.tcpsession.send],"<<LogSelf()<<FUN_FILE_LINE);
				return 9090102;
			}

			//bool ismsfssc = this->IsXMark(socketmk_isclose_sendfail)&&this->IsXMark(socketmk_server_client);
			if( data_size >= m_send_buf.get_buffer_maxlen() )
				return 9090103;

			bool send_buf_is_empty = m_send_buf.IsEmpty();
			if( !m_send_buf.push(data_ptr,(unsigned)data_size) )
			{
				NETLOG_ERROR(LogSelf()<<"��������,����ʧ��,connecid="<<m_conn_index<<";"<<m_send_buf.get_buffer_info() );
				if( this->IsXMark(socketmk_isclose_sendfail) )
				{
					//����ǿͻ��˵�����
					if( this->IsXMark(socketmk_server_client) /*&& (0==m_conn_index%50)*/ )
					{
						vector<tcp_pak_header> vecmsgheads;
						m_send_buf.debug_print_buffermsglist(m_conn_index,vecmsgheads);
						//WriteNetBufferMsgListInfo(m_conn_index,vecmsgheads );
					}

					NETLOG_ERROR("[net.tcpsession.send]�ر�����,��������,����ʧ��;connecid:"<<m_conn_index<<"; "<<m_send_buf.get_buffer_info());
					CloseSocketOut("SendBufFill!");
				}
				return 9090104;
			}
			m_pStatData->SendLastSecFlow.uValue += (unsigned)data_size;
			m_pStatData->SendLastMinFlow.uValue += (unsigned)data_size;

			//	async. send loop has been started, just return
			if( !send_buf_is_empty )
				return 0;

			unsigned uData_len 	= 0;
			char    *pSend		= (char*)m_send_buf.front(uData_len);

			bool ishappend_error		= false;
			tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
			if( ((unsigned)ppak_header->GetLen()) > m_send_buf.get_buffer_maxlen())
			{
				//���գ����ĳ��ȱ��������������󣬺����԰�ͷ����
				ishappend_error = true;
				NETLOG_ERROR("[���ݰ�ͷ����,���ر�����]<6> ��ͷ̫��! pHeader->Len="<<ppak_header->GetLen()<<",conn_index="<<GetConnectIndex()<<FUN_FILE_LINE);
			}

			if( ppak_header->GetLen()<sc_pak_header_len )
			{
				//���գ����ĳ��ȱȰ�ͷ��С�����Դ���
				ishappend_error = true;
				NETLOG_ERROR("[���ݰ�ͷ����,���ر�����]<8> ��ͷ̫С[<12]! pHeader->Len="<<ppak_header->GetLen()<<LogSelf() );
			}

			if( !ppak_header->check() )
			{
				NETLOG_ERROR("[���ݰ�ͷ����,���ر�����]<10> pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<";CheckCode="<<ppak_header->getCheckCode() );
				ishappend_error = true;
			}
			if( ishappend_error )
			{
				CloseSocketOut("SendLogicError!");
				return 9090105;
			}

			if( SocketSendOneMsg((char*)pSend,uData_len)>0 )
			{
				CloseSocketOut("SendMsgExceptionAbb!");
				return 652101;
			}

			return 0;
		}
		void CWSSocketBase::XBase_WriteCb(const ZBoostErrCode& error,int iDataType)
		{
			if( error )
			{
				if( this->IsXMark(socketmk_log) ){
					NETLOG_ERROR(LogSelf()<<", error="<<error.message()<<",errorid="<<error.value()<<FUN_FILE_LINE );
				}
				CloseSocketOut("HandleReadError!");
				return;
			}

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

			if( SocketSendOneMsg((char*)pSend,uData_len)>0 )
			{
				CloseSocketOut("SendMsgExceptionAaa!");
				return;
			}

			m_LastLiveTm = PNGB::m_server_curTm;
			if( PNGB::m_pLog->IsMayLog(Log_DebugNet) )
			{
				if( this->IsXMark(socketmk_server_client) && this->IsXMark(socketmk_log) )
				{
					NETLOG_DBNET("[SendPack]; ConID="<<GetConnectIndex()<<";SendPack["<<ppak_header->GetMsgID()<<";"<<ppak_header->GetLen()<<";"<<"]");
				}
			}
		}
		//Socket ����һ��������Ϣ
		int  CWSSocketBase::SocketSendOneMsg( char *pSend,unsigned uData_len )
		{
			TCurMsgHead tTmepHead;
			tTmepHead.Reset();
			tTmepHead.HeadBuffer[0] = *(pSend+sc_pak_header_len);
			tTmepHead.HeadBuffer[1] = *(pSend+sc_pak_header_len+1);
			tTmepHead.AnalyMsgHeadBuf();
			//NETLOG_SYSINFO("[����WebSocket��Ϣ -->>] : "<<tTmepHead.LogInfo() );
			
			uData_len = uData_len-sc_pak_header_len;
			char *pTtBuf = (char*)( pSend+sc_pak_header_len );
			int iErrID = this->XBase_WriteData( pTtBuf,uData_len,uData_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[async_write�쳣]:"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				return 65210141;
			}
			return 0;
		}
		//������յ�����Ϣ
		bool CWSSocketBase::Run(unsigned uCount)
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
			if( (data_len<sc_pak_header_len) || (data_len>=m_recv_buf.get_buffer_maxlen()) )
			{
				NETLOG_FATAL("[��Ϣ��С�쳣�������ر�����,net],"<<LogSelf()<<FUN_FILE_LINE );
				this->CloseSocketOut("MsgLenError!");
				return false;
			}
			INPROTOCOL::TInNiceNetHead *pHeadBase = (INPROTOCOL::TInNiceNetHead *)pBuffer;
			recvbuf_lock.unlock();
			try
			{
				m_pStatData->RecLastSecFlow.uValue += data_len;
				m_pStatData->RecLastMinFlow.uValue += data_len;

				if( false==DoNetInCmd(GetConnectIndex(),pBuffer,data_len) )
				{
					if( !PNGB::IsXMarked(EmZhtCon_NotRunMsg) )
					{
						char *pMsgBody = (char*)pBuffer + sizeof(INPROTOCOL::TInNiceNetHead);
						//NETLOG_SYSINFO("[�յ�һ����ϢWS].... "<<LogSelf()<<"; OPCode="<<pHeadBase->GetMsgID()<<", Msg="<<pMsgBody );	
						this->OnRecive( GetConnectIndex(),pBuffer,data_len );
					}else{
						//NETLOG_NORMAL("[��Ϣ����.net] lostmsg,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
					}
				}
			}catch(...)
			{
				NETLOG_FATAL("[��Ϣ�����쳣.net.WebSocket],"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->GetLen()<<"] data_len["<<data_len<<"]" );	
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
			}

			recvbuf_lock.lock();
			m_recv_buf.pop();
			if(m_recv_buf.m_IsStopRec )
			{
			}
			return true;
		}
		string InFunBuildWsSocketMsg(){
			Json::Value jsMsgHead;
			Json::Value jsCloseMsg;
			jsMsgHead["MsgID"]    = 0;
			jsCloseMsg["MsgHead"] = jsMsgHead;
			jsCloseMsg["MsgBody"] = "{\"DescAa\":\"CcReqCloseWS.ForPeyone!\"}";
			Json::FastWriter json_writer;
			string  strJsonObj=json_writer.write(jsCloseMsg);

			char desbuffer[1024]={0};
			Base64::Base64_Encode(desbuffer,strJsonObj.c_str(), (unsigned)strJsonObj.size());
			return string(desbuffer);
		}
		bool CWSSocketBase::DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len)
		{
			if( data_len<g_MsgHeadLen )
				return false;
			INPROTOCOL::TInNiceNetHead *pMsgHead = (INPROTOCOL::TInNiceNetHead *)pdata;
			unsigned iMsgType = pMsgHead->GetWebosockDataType();
			if( 0X8==iMsgType )
			{//�ͻ�������ر�����
				NETLOG_SYSINFO("[Wss�ͻ�������ر�����] connectid="<<connectid<<FUN_FILE_LINE );
				//string strWhyClose = "Client reqCloseWebSocket!!";
				string strWhyClose=InFunBuildWsSocketMsg();
				this->SendMsg_Aaa( strWhyClose.c_str(),(unsigned)strWhyClose.size(),0X81 );
				this->inclose("CcReqCloseWSocket!");
				return true;
            }
            else if( 0X9==iMsgType )
            {//ϵͳ������Ϣ
                SendPongMsgToCc( connectid,pMsgHead );
                return true;
            }

			char chXxInCmd[128]={ 0 };
			if(pMsgHead->GetLen()<sizeof(chXxInCmd)-10)
			{
				memcpy(chXxInCmd,(char*)pdata+g_MsgHeadLen,data_len-g_MsgHeadLen);
			}
			if(string("@heart")==string(chXxInCmd))
			{//������Ϣ
				return true;
			}

			return false;
		}

        void CWSSocketBase::SendPongMsgToCc(unsigned connectid,INPROTOCOL::TInNiceNetHead *pMsgPing)
        {
            unsigned   iMsgBodyLen = (int)(pMsgPing->GetLen()-g_MsgHeadLen);
            const char *pWssData   = (char*)pMsgPing+g_MsgHeadLen;
            NETLOG_SYSINFO("[Wss�ͻ���Ping] connectid="<<connectid<<";iMsgBodyLen="<<iMsgBodyLen<<"; msgInfo:"<<pWssData );

            string strCcMsg = PNGB::m_NetExFunMg->funUTF8ToGBK( pWssData,(unsigned)strlen(pWssData) );
            string strPongMsg = "MmPreGt!";
            strPongMsg = PNGB::m_NetExFunMg->funGBKToUTF8(strPongMsg.c_str(), (unsigned)strPongMsg.size());
            this->SendMsg_Aaa(strPongMsg.c_str(), (unsigned)strPongMsg.size(), 0X8A);
        }

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	}
}
