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
			//<1>判断是否发生了错误
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
				//下面的三个是一直循环的
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
			//开始一次读操作
			this->BeginOnceReadLogicMsg();
		}

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		bool CWSSocketBase::BeginOnceReadLogicMsg()
		{
			//判断包头是否读完成，
			if( false == m_MsgHead.HeadRecOver )
			{
				if( false == m_MsgHead.HearFirstIsRead )
				{//判断包头的前两个字节是否读取
					m_MsgHead.HearFirstIsRead = true;
					ImpRecMaxLenMsg( EHReadType_LogicMsgHeadFirst, m_MsgHead.HeadBuffer,2,2 );
				}else
				{//读包头剩下的部分
					if( m_MsgHead.HeadSecondPartLen<1 || m_MsgHead.HeadSecondPartLen>12 )
					{
						this->CloseSocketOut("WebCalHeadSecondLen Error...");
						return false;
					}

					//标记读包头完成的标记
					m_MsgHead.HeadRecOver = true;
					//读包头的第二部分
					ImpRecMaxLenMsg( EHReadType_LogicMsgHeadSecond,m_MsgHead.HeadBuffer+2,m_MsgHead.HeadSecondPartLen,m_MsgHead.HeadSecondPartLen );
				}
			}else
			{//读包体
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

			//设置系统参数
			m_recv_buf.m_IsStopRec = false;

			//计算临时包头
			tcp_pak_header &tMsgHead = recv_buf->m_RHeader;
			tMsgHead.reset();
			tMsgHead.SetLen( m_MsgHead.uMsgLength+sc_pak_header_len );
			tMsgHead.SetWebosockDataType( m_MsgHead.iMsgType );

			//拷贝包头
			memcpy(pBuffer, &tMsgHead, sc_pak_header_len );

			if( 0==m_MsgHead.uMsgLength )
			{//这个数据包没有包体
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
			// 	数据包类型（frame type），占4bits
			// 	0x0：标识一个中间数据包
			// 	0x1：标识一个text类型数据包
			// 	0x2：标识一个binary类型数据包
			// 	0x3-7：保留
			// 	0x8：标识一个断开连接类型数据包
			// 	0x9：标识一个ping类型数据包
			// 	0xA：表示一个pong类型数据包
			// 	0xB-F：保留

			unsigned char chFirstByte = (unsigned char)m_MsgHead.HeadBuffer[0];
			//最高位用于描述消息是否结束,如果为1则该消息为消息尾部,如果为零则还有后续数据包
			m_MsgHead.IsMsgEof = !!(chFirstByte>>7);
			//判断消息类型
			m_MsgHead.iMsgType = chFirstByte&(0XF);
			unsigned char chSecondByte = (unsigned char)m_MsgHead.HeadBuffer[1];
			//判断是否又掩码处理
			m_MsgHead.IsHaseMask = !!(chSecondByte>>7);
			if( m_MsgHead.IsHaseMask )
				m_MsgHead.HeadSecondPartLen = 4;

			if( 0X8==m_MsgHead.iMsgType )
			{
				int iXss = 0;
				iXss++;
            }
            else if(0X9==m_MsgHead.iMsgType)
            {//标识一个ping类型数据包
                int iXss = 0; iXss++;
            }
			//NETLOG_SYSINFO("Wss, 包类型...iMsgType="<<m_MsgHead.iMsgType<<FUN_FILE_LINE);

			//消息的长度
			m_MsgHead.uMsgLength = (unsigned)(chSecondByte & 0x7F);
			if( 0==m_MsgHead.uMsgLength )
			{//只有包头，没有消息包体,这个消息完成了
				//包头读完成了，
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
			{//这个包的包头就2个字节，并且没有掩码
				m_MsgHead.HeadRecOver = true;
				if( 0==m_MsgHead.uMsgLength )
				{
					//如果这个包的长度为0，就只有包头的两个字节
					//丢弃这个包吧，这里没有什么鸟用
					NETLOG_ERROR("[收到了一个空包]"<<FUN_FILE_LINE );
					m_MsgHead.Reset();
				}else
				{
					//还有一部分包体，那么就接收包体吧
				}
			}else
			{
				//没有什么要处理的，继续接收包体吧
			}
		}
		void CWSSocketBase::HandRead_LogicMsgHeadSecond( size_t bytes_transferred )
		{
			
			if( 126>m_MsgHead.uMsgLength )
			{
				//获取掩码
				if( m_MsgHead.IsHaseMask )
					memcpy( m_MsgHead.chMask,&m_MsgHead.HeadBuffer[2],sizeof(m_MsgHead.chMask) );
			}else if( 126==m_MsgHead.uMsgLength )
			{
				//获取消息包体的真实长度
				m_MsgHead.uMsgLength = ntohs( *(unsigned short*) (m_MsgHead.HeadBuffer+2) );
				if( m_MsgHead.IsHaseMask )
					memcpy( m_MsgHead.chMask,&m_MsgHead.HeadBuffer[4],sizeof(m_MsgHead.chMask) );
			}else
			{
				NETLOG_ERROR(FUN_FILE_LINE);
			}

			if( 0==m_MsgHead.uMsgLength )
			{
				//如果这个包的长度为0，就只有包头的两个字节
				//丢弃这个包吧，这里没有什么鸟用
				NETLOG_ERROR("[收到了一个空包aa]"<<FUN_FILE_LINE );
				m_MsgHead.Reset();
			}

			//NETLOG_SYSINFO("[ 收到WebSocket消息 <<-- ] : "<<m_MsgHead.LogInfo()<<FUN_FILE_LINE );
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
				//进行解码
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
				NETLOG_ERROR("[net.异常..1]"<<LogSelf()<<FUN_LINE);
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
		//push一个消息到发送缓冲区
		int  CWSSocketBase::SendMsg(const char *pdata,unsigned data_len)
		{
			return this->SendMsg_Aaa( pdata,data_len,0X81 );
		}

		int CWSSocketBase::SendWebSocketMsg(const void *pdata,unsigned data_len,bool IsText)
		{
            int  iDataType = IsText?0X81:0X82;
			return this->SendMsg_Aaa((const char*)pdata,data_len,iDataType );
		}

		//push一个消息到发送缓冲区
        //iDataType( 0X81[一个文本数据包]; 0X82[一个二进制数据包]; 0X8A(一个pong数据包) )
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

			//生成发送缓冲区的包头
			int  iCurBufPos = sc_pak_header_len;
			tcp_pak_header *pMsgHead = (tcp_pak_header*)g_WebSendBuf;

			//写包头的第一个固定字节
			//unsigned char chFirstByte = 0X81(文本) 0X82(二进制);
			//unsigned char chFirstByte = IsText?0X81:0X82;
            unsigned char chFirstByte = iDataType;
            g_WebSendBuf[iCurBufPos] = chFirstByte;
			iCurBufPos += 1;

			//写包头的长度第二个字节
			unsigned short uTureMsgLen = data_len;
			if( data_len<126 )
			{
				g_WebSendBuf[iCurBufPos] = (unsigned char)data_len;
				iCurBufPos += 1;
			}else if( data_len<65000 )
			{
				g_WebSendBuf[iCurBufPos] = 126;
				iCurBufPos += 1;
				//写长度
				uTureMsgLen = ntohs(uTureMsgLen);
				memcpy( &g_WebSendBuf[iCurBufPos],&uTureMsgLen,sizeof(uTureMsgLen) );
				iCurBufPos += 2;
			}

			if( 0 /*uMask>0*/ )
			{
				unsigned uMask = 34242;
				//写掩码
				memcpy( &g_WebSendBuf[iCurBufPos],&uMask,sizeof(uMask) );
				iCurBufPos += sizeof(uMask);

				//加密数据
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
				//写数据
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
				NETLOG_ERROR(LogSelf()<<"缓冲区满,发送失败,connecid="<<m_conn_index<<";"<<m_send_buf.get_buffer_info() );
				if( this->IsXMark(socketmk_isclose_sendfail) )
				{
					//如果是客户端的链接
					if( this->IsXMark(socketmk_server_client) /*&& (0==m_conn_index%50)*/ )
					{
						vector<tcp_pak_header> vecmsgheads;
						m_send_buf.debug_print_buffermsglist(m_conn_index,vecmsgheads);
						//WriteNetBufferMsgListInfo(m_conn_index,vecmsgheads );
					}

					NETLOG_ERROR("[net.tcpsession.send]关闭连接,缓冲区满,发送失败;connecid:"<<m_conn_index<<"; "<<m_send_buf.get_buffer_info());
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
				//我日，包的长度比整个缓冲区还大，很明显包头错误
				ishappend_error = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接]<6> 包头太长! pHeader->Len="<<ppak_header->GetLen()<<",conn_index="<<GetConnectIndex()<<FUN_FILE_LINE);
			}

			if( ppak_header->GetLen()<sc_pak_header_len )
			{
				//我日，包的长度比包头还小，明显错误
				ishappend_error = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接]<8> 包头太小[<12]! pHeader->Len="<<ppak_header->GetLen()<<LogSelf() );
			}

			if( !ppak_header->check() )
			{
				NETLOG_ERROR("[数据包头错误,将关闭连接]<10> pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<";CheckCode="<<ppak_header->getCheckCode() );
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
				//我日，包的长度比整个缓冲区还大，很明显包头错误
				ishappend_error = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接] 包头太长! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_FILE_LINE);
			}

			if( ppak_header->GetLen()<sc_pak_header_len )
			{
				//我日，包的长度比包头还小，明显错误
				ishappend_error = true;
				NETLOG_ERROR("[数据包头错误,将关闭连接] 包头太小[<12]! pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<FUN_FILE_LINE);
			}

			if( !ppak_header->check() )
			{
				NETLOG_ERROR("[数据包头错误,将关闭连接] pHeader->Len="<<ppak_header->GetLen()<<LogSelf()<<";CheckCode="<<ppak_header->getCheckCode()<<FUN_FILE_LINE);
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
		//Socket 发送一个网络消息
		int  CWSSocketBase::SocketSendOneMsg( char *pSend,unsigned uData_len )
		{
			TCurMsgHead tTmepHead;
			tTmepHead.Reset();
			tTmepHead.HeadBuffer[0] = *(pSend+sc_pak_header_len);
			tTmepHead.HeadBuffer[1] = *(pSend+sc_pak_header_len+1);
			tTmepHead.AnalyMsgHeadBuf();
			//NETLOG_SYSINFO("[发送WebSocket消息 -->>] : "<<tTmepHead.LogInfo() );
			
			uData_len = uData_len-sc_pak_header_len;
			char *pTtBuf = (char*)( pSend+sc_pak_header_len );
			int iErrID = this->XBase_WriteData( pTtBuf,uData_len,uData_len,0 );
			if( iErrID>0 )
			{
				NETLOG_ERROR("[async_write异常]:"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				return 65210141;
			}
			return 0;
		}
		//处理接收到的消息
		bool CWSSocketBase::Run(unsigned uCount)
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
			void    *pBuffer  = m_recv_buf.front(data_len);
			if( (data_len<sc_pak_header_len) || (data_len>=m_recv_buf.get_buffer_maxlen()) )
			{
				NETLOG_FATAL("[消息大小异常，主动关闭连接,net],"<<LogSelf()<<FUN_FILE_LINE );
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
						//NETLOG_SYSINFO("[收到一个消息WS].... "<<LogSelf()<<"; OPCode="<<pHeadBase->GetMsgID()<<", Msg="<<pMsgBody );	
						this->OnRecive( GetConnectIndex(),pBuffer,data_len );
					}else{
						//NETLOG_NORMAL("[消息处理.net] lostmsg,"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->Len<<"] data_len["<<data_len<<"]" );	
					}
				}
			}catch(...)
			{
				NETLOG_FATAL("[消息处理异常.net.WebSocket],"<<LogSelf()<<" OPCode["<<pHeadBase->GetMsgID()<<"] pHeadBase->Len["<<pHeadBase->GetLen()<<"] data_len["<<data_len<<"]" );	
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
			{//客户端请求关闭链接
				NETLOG_SYSINFO("[Wss客户端请求关闭链接] connectid="<<connectid<<FUN_FILE_LINE );
				//string strWhyClose = "Client reqCloseWebSocket!!";
				string strWhyClose=InFunBuildWsSocketMsg();
				this->SendMsg_Aaa( strWhyClose.c_str(),(unsigned)strWhyClose.size(),0X81 );
				this->inclose("CcReqCloseWSocket!");
				return true;
            }
            else if( 0X9==iMsgType )
            {//系统心跳消息
                SendPongMsgToCc( connectid,pMsgHead );
                return true;
            }

			char chXxInCmd[128]={ 0 };
			if(pMsgHead->GetLen()<sizeof(chXxInCmd)-10)
			{
				memcpy(chXxInCmd,(char*)pdata+g_MsgHeadLen,data_len-g_MsgHeadLen);
			}
			if(string("@heart")==string(chXxInCmd))
			{//心跳消息
				return true;
			}

			return false;
		}

        void CWSSocketBase::SendPongMsgToCc(unsigned connectid,INPROTOCOL::TInNiceNetHead *pMsgPing)
        {
            unsigned   iMsgBodyLen = (int)(pMsgPing->GetLen()-g_MsgHeadLen);
            const char *pWssData   = (char*)pMsgPing+g_MsgHeadLen;
            NETLOG_SYSINFO("[Wss客户端Ping] connectid="<<connectid<<";iMsgBodyLen="<<iMsgBodyLen<<"; msgInfo:"<<pWssData );

            string strCcMsg = PNGB::m_NetExFunMg->funUTF8ToGBK( pWssData,(unsigned)strlen(pWssData) );
            string strPongMsg = "MmPreGt!";
            strPongMsg = PNGB::m_NetExFunMg->funGBKToUTF8(strPongMsg.c_str(), (unsigned)strPongMsg.size());
            this->SendMsg_Aaa(strPongMsg.c_str(), (unsigned)strPongMsg.size(), 0X8A);
        }

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	}
}
