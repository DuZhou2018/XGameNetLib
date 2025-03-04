/*****************************************************************************************************************************
    Created:   2017/8/28
    Author:    ZhangHongtao,   1265008170@qq.com
    FileName:  G:\ZhtXGame\PeonyNetWebSocket\header\TCPXSessionBase.hpp
 
******************************************************************************************************************************/
#ifndef XZhtTCPBase_h__
#define XZhtTCPBase_h__

#include "./OmpConfig.hpp"
#include "./QueueSafe.h"
#include "./CGlobal.hpp"
#include "./INiceNetServer.h"
#include "./Scheduler_impl.hpp"
#include "./ClientServerIDMG.h"

#include "../header/NiceLog.h"
#include "../header/NiceIpLimit.h"
#include "../include/INiceNetCounter.h"

namespace peony 
{
    namespace net 
	{
		class   TCPClientSession;
		class   WSClientSession;
		class   TCPServerSession;
		class   HttpServerSession;
		class   WebSocketSession;
		typedef boost::shared_ptr<TCPClientSession>  TCPClientSessionPtr;
		typedef boost::shared_ptr<WSClientSession>   WSClientSessionPtr;
		typedef boost::shared_ptr<TCPServerSession>  TCPServerSessionPtr;
		typedef boost::shared_ptr<HttpServerSession> HttpServerSessionPtr;
		typedef boost::shared_ptr<WebSocketSession>  WebSocketSessionPtr;
		extern void WriteNetBufferMsgListInfo(unsigned ConID, vector<tcp_pak_header> &vecmsgheads);

		template < typename T >
        class XTcpSocketBase : public INiceNetSocket
        {
        public:
			XTcpSocketBase( io_context &io_service,TXZTBaseInitPm &initPM);
            virtual ~XTcpSocketBase();

		public: //INiceNetSocket 接口部分
			virtual void		StartSocket(){}
			//这个函数只是关闭网络连接，可能会出发读，写错误发生，而回调inclose去完成关闭，删除
			virtual void		CloseSocketOut(string strwhy);
			virtual TcpSocket*  GetTcpSocket(){ return &m_socket; }
			virtual bool		IsGoodConnect() { return m_IsConnectOk; } //网络是否可以使用

		public:
			string			GetRemoteIp(unsigned &uPort);
			string			GetRemoteIp();
			string			GetLocalIp(unsigned &uPort);
			string			GetLocalIp();

			unsigned		get_lastlive_time() { return m_LastLiveTm; }
			void			GetSessionBufferInfo(TSessionBufferInfo &info);
			void			ClearSeeionBufferInfo();
			bool		    IsAllocSendRecBuffer(){ return (m_recv_buf.isok_alloc_buffer()&&m_send_buf.isok_alloc_buffer()); }

		protected:
			//这个函数完成回调操作，感觉这个函数不应该调用CloseSocketOut
			void			inclose( string strwhy="DefaultCurBy" );
			void		    OpenState();

		protected:
			//网络初始化完成后，调用这个函数启动接收消息的工作
			virtual bool    XxTcpBeginRecWork()=0;
			virtual void	XBase_ReadCb( const ZBoostErrCode& error,size_t bytes_transferred,int iDataType ){}
			virtual void	XBase_WriteCb(const ZBoostErrCode& error,int iDataType){}

            int             Tcp_in_do_handle_read(size_t bytes_transferred);
            bool            Tcp_read_packet_body();

		protected:
			/*****************************************************************************************************************************
				Created:   2017/9/12
				Author:    ZhangHongtao,   1265008170@qq.com 
				    
				提供一个通用的读写网络数据的接口;
				iDataType 可以理解读写操作发起的位置，回调读写事件的时候会回调上去
			******************************************************************************************************************************/
			int             XBase_ReadData(  const char *pBuffer,unsigned uMaxLen,unsigned uMinLen, int iDataType );
			int             XBase_WriteData( const char *pBuffer,unsigned uMaxLen,unsigned uMinLen, int iDataType );
		protected:
			TcpSocket			        m_socket;

			// buffers
			boost::recursive_mutex		m_sendbuf_mutex;		 //	for m_send_buf's res. protection
			boost::recursive_mutex		m_recvbuf_mutex;		 //	for m_recv_buf's res. protection
			RecvBufferType				m_recv_buf;
			SendBufferType				m_send_buf;
		};

		template < typename T >
		peony::net::XTcpSocketBase<T>::XTcpSocketBase(io_context &io_service,TXZTBaseInitPm &initPM )
				:m_socket( io_service ),
				 m_recv_buf(initPM.r_alloc,initPM.r_size),
				 m_send_buf(initPM.s_alloc,initPM.s_size)
		{
			m_pStatData    = initPM.pStatData;
			m_InFunConnect = initPM.mFunConnect;
			m_uXServerID   = initPM.mXServerID;
			this->ClearAllXMark();
			m_uAPPEData = 0;
			//发送失败关闭连接
			this->SetXMark( socketmk_isclose_sendfail );
            this->SetXMark( socketmk_NoCheckHeart );
			//m_xstmark |= socketmk_log;
			memset(m_SocketTypeName, 0, sizeof(m_SocketTypeName));
			memset(m_SocketWhyClose,0,sizeof(m_SocketWhyClose) );
			memset(m_selflog,0,sizeof(m_selflog) );
			strcpy(m_selflog," HttpSession<T>::HttpSession; " );
			m_IsConnectOk = false;
			m_LastLiveTm  = 0;
			m_pMyXServer  = XServerSPtr();

			CInPubFun::strcpy(m_RomeIp,sizeof(m_RomeIp),"RomeIpIsZero!",903231);
		}

		template < typename T >
		peony::net::XTcpSocketBase<T>::~XTcpSocketBase()
		{
			string strRoIp = this->GetRemoteIp();
			CIpLimitMg::Instance().DelOneIp(strRoIp);
		}

		template < typename T >
		string peony::net::XTcpSocketBase<T>::GetRemoteIp(unsigned &uPort)
		{
			uPort = 0;
			ZBoostErrCode ec;
			boost::asio::ip::tcp::endpoint remoteIp = m_socket.remote_endpoint(ec);
			if( ec )
			{
				return m_RomeIp;
			}

			uPort = remoteIp.port();
			string strRrIp = remoteIp.address().to_string();
			CInPubFun::strcpy(m_RomeIp,sizeof(m_RomeIp),strRrIp.c_str(),903238 );
			return strRrIp;
		}

		template < typename T >
		string peony::net::XTcpSocketBase<T>::GetRemoteIp()
		{
			unsigned uport=0;
			return GetRemoteIp( uport );
		}

		template < typename T >
		string peony::net::XTcpSocketBase<T>::GetLocalIp(unsigned &uPort)
		{
			uPort = 0;
			ZBoostErrCode ec;
			boost::asio::ip::tcp::endpoint remoteIp = m_socket.local_endpoint(ec);
			if( ec )
				return "null";

			uPort = remoteIp.port();
			return remoteIp.address().to_string();            

		}

		template < typename T >
		string peony::net::XTcpSocketBase<T>::GetLocalIp()
		{
			unsigned uport=0;
			return GetLocalIp( uport );
		}

		template < typename T >
		void peony::net::XTcpSocketBase<T>::CloseSocketOut(string strwhy)
		{
			m_IsConnectOk = false;
			ZBoostErrCode ec;
			if( !m_socket.is_open() )
				return;

			if( strlen(m_SocketWhyClose)<3 )
			{//保存最初的关闭的原因
				string strTtWhy = "Me_"+strwhy;
				CInPubFun::strcpy( m_SocketWhyClose,sizeof(m_SocketWhyClose),strTtWhy.c_str(),930201 );
			}

			try
			{
				if( this->IsXMark(socketmk_log) || this->IsXMark(socketmk_logic_iniserver)  ){
					NETLOG_SYSINFO("[close] strwhy="<<strwhy<<LogSelf() );
				}

				//for gracefull
				m_socket.shutdown(TcpSocket::shutdown_both,ec);
				if( ec && this->IsXMark(socketmk_log) ){
					NETLOG_WARNING("[net.tcpsession.close 异常]:"<<LogSelf()<<ec.message()<<FUN_FILE_LINE);
				}
			}catch(...){
				NETLOG_ERROR("[net.tcpsession.close 异常]:"<<LogSelf()<<FUN_FILE_LINE);
			}
		}

		template < typename T >
		void peony::net::XTcpSocketBase<T>::inclose(string strwhy/*="DefaultCurBy" */)
		{
			CloseSocketOut(strwhy);
			Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
			if( this->IsXMark(socketmk_tcpsession_indelq) )
				return;
			this->SetXMark( socketmk_tcpsession_indelq );

			string strWhyDesc = strwhy;
			if( this->IsXMark(socketmk_IsHeartTimeout) ){
				strWhyDesc = boost::str( boost::format("%s_%s_%s !")%m_SocketTypeName%strwhy%"HeartTimeOut" );
			}else{
				strWhyDesc = boost::str( boost::format("%s_%s_%s !")%m_SocketTypeName%strwhy%"NoZhtPos" );
			}

			mainrun_call_onconnected(m_uXServerID, this->m_conn_index,Both_SelfCloseConnect,strWhyDesc,shared_from_this() );
		}

		template < typename T >
		void peony::net::XTcpSocketBase<T>::GetSessionBufferInfo(TSessionBufferInfo &info)
		{
			if(1)
			{
				Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
				info.uSendUsedPercent = m_send_buf.get_sendbuffer_info(info.uSendOKCount,info.uSendFailCount);
			}
			if(1)
			{
				Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
				info.uReciveUsedPercent = m_recv_buf.get_recivebuffer_info(info.uReciveCount);
			}

		}

		template < typename T >
		void peony::net::XTcpSocketBase<T>::ClearSeeionBufferInfo()
		{
			if(1)
			{
				Boost_Scoped_Lock sendbuf_lock(m_sendbuf_mutex);
				m_send_buf.clear_send_counterinfo();
			}
			if(1)
			{
				Boost_Scoped_Lock sendbuf_lock(m_recvbuf_mutex);
				m_recv_buf.clear_recive_counterinfo();
			}

		}


		template < typename T >
		void peony::net::XTcpSocketBase<T>::OpenState()
		{
			m_IsConnectOk = true;
			this->SetXMark( socketmk_IsOpenState );
			m_LastLiveTm = PNGB::m_server_curTm;
			//Socket option for disabling the Nagle algorithm.
			boost::asio::ip::tcp::no_delay option(true);
			m_socket.set_option(option);
			
			//启动接收消息的工作
			XxTcpBeginRecWork();
		}

		template < typename T >
		int peony::net::XTcpSocketBase<T>::XBase_ReadData(const char *pBuffer,unsigned uMaxLen,unsigned uMinLen, int iDataType)
		{
			try
			{
				boost::asio::async_read( m_socket,
 	  									 boost::asio::buffer( (char*)pBuffer,uMaxLen ),
										 boost::asio::transfer_at_least(uMinLen),
										 boost::bind( &INiceNetSocket::XBase_ReadCb,
										 			   boost::enable_shared_from_this<INiceNetSocket>::shared_from_this(),
													   boost::asio::placeholders::error,
													   boost::asio::placeholders::bytes_transferred,iDataType ));
			}
			catch(...)
			{
				NETLOG_ERROR("[net.异常..1]"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("ReadMsgBodyException!");
				return 99908801;
			}

			return 0;
		}

		template < typename T >
		int peony::net::XTcpSocketBase<T>::XBase_WriteData(const char *pBuffer,unsigned uMaxLen,unsigned uMinLen, int iDataType)
		{
			try
			{
				bool IsOpen = m_socket.is_open();
				boost::asio::async_write( 
					m_socket,
					boost::asio::buffer(pBuffer,uMaxLen),
					boost::asio::transfer_at_least(uMinLen),
					boost::bind( &INiceNetSocket::XBase_WriteCb,
					              boost::enable_shared_from_this<INiceNetSocket>::shared_from_this(),
								  boost::asio::placeholders::error,iDataType ));
			}
			catch (...)
			{
				NETLOG_ERROR("[async_write异常]:"<<LogSelf()<<FUN_FILE_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				return 99908802;
			}
			return 0;
		}

        template < typename T >
        int  peony::net::XTcpSocketBase<T>::Tcp_in_do_handle_read(size_t bytes_transferred)
        {
            /********************************************************************
            created:	5:11:2010   9:42
            author:		zhanghongtao
            purpose:
            return:  -1, error;
            1, ok, begin read head
            2. ok, finish one msg;
            *********************************************************************/

            //	deny potential re-entered behavior
            Boost_Scoped_Lock recvbuf_lock(m_recvbuf_mutex);
            RecvBufferType * recv_buf = &m_recv_buf;

            tcp_pak_header *pDdRHeader = &recv_buf->m_RHeader;
            if(recv_buf->m_pak_header_readed == false)
            {
                //pak-header has been readed
                //assert( bytes_transferred == sc_pak_header_len );
                if(bytes_transferred != sc_pak_header_len)
                {
                    NETLOG_ERROR("[net.tcpsession.长度错误]:"<<LogSelf()<<"OPCode:"<<pDdRHeader->GetMsgID()<<"; Len:"<<pDdRHeader->GetLen()<<FUN_FILE_LINE);
                    return -1;
                }
                //	check
                if(recv_buf->m_RHeader.check() == false)
                {
                    NETLOG_ERROR("[net.tcpsession.效验错误]:"<<LogSelf()<<"OPCode:"<<pDdRHeader->GetMsgID()<<"; Len:"<<pDdRHeader->GetLen()<<";CheckCode="<<pDdRHeader->getCheckCode()<<FUN_FILE_LINE);
                    return -1;
                }
                recv_buf->m_pak_header_readed = true;

                //char *pbuffer =  (char*)&recv_buf->m_RHeader;
                size_t body_len = recv_buf->m_RHeader.GetLen();
                if(body_len > (recv_buf->get_buffer_maxlen()-sc_pak_header_len))
                {
                    NETLOG_ERROR("[net.tcpsession.包头错误]:"<<LogSelf()<<"OPCode:"<<pDdRHeader->GetMsgID()<<"; Len:"<<pDdRHeader->GetLen()<<FUN_FILE_LINE);
                    return -1;
                }

                if(PNGB::m_pLog->IsMayLog(Log_DebugNet))
                {
                    if(this->IsXMark(socketmk_server_client) && this->IsXMark(socketmk_log))
                    {
                        NETLOG_DBNET("[ReadPackHead] ConID="<<this->GetConnectIndex()<<",RecivePack:["<<pDdRHeader->GetMsgID()<<";"<<pDdRHeader->GetLen()<<"]");
                    }
                }

                // read body
                if(Tcp_read_packet_body())
                    return 1;
                else
                    return -1;
            }
            else
            {
                //pak-body has been readed
                size_t body_len = recv_buf->m_RHeader.GetLen()-sc_pak_header_len;
                if(bytes_transferred != body_len)
                {
                    NETLOG_ERROR("read msgbody error! m_conn_index="<<m_conn_index<<",bytes_transferred="<<bytes_transferred<<",body_len="<<body_len<<FUN_FILE_LINE);
                    return -1;
                }
                if(!recv_buf->finish_push((unsigned)(bytes_transferred+sc_pak_header_len)))
                {
                    NETLOG_ERROR(__FUNCTION__<<" connecid:"<<m_conn_index<<";finish_push() fail!");
                    return -1;
                }

                //if( PNGB::m_pLog->IsMayLog(Log_DebugNet) )
                //{
                //	if( this->IsXMark(socketmk_server_client) && this->IsXMark(socketmk_log) )
                //	{
                //		NETLOG_DBNET("[ReadPackFinish] ConID="<<this->GetConnectIndex()<<",RecivePack:["<<pDdRHeader->OPCode<<";"<<pDdRHeader->Len<<";"<<pDdRHeader->TransID<<"]" );
                //	}
                //}
                return 2;
            }
        }

        template < typename T >
        bool peony::net::XTcpSocketBase<T>::Tcp_read_packet_body()
        {
            RecvBufferType * recv_buf = &m_recv_buf;
            // read body
            unsigned body_len = recv_buf->m_RHeader.GetLen()-sc_pak_header_len;
            void *pBuffer = recv_buf->open_for_push(recv_buf->m_RHeader.GetLen());
            if(pBuffer)
            {
                m_recv_buf.m_IsStopRec = false;
                //拷贝包头
                memcpy(pBuffer, &recv_buf->m_RHeader, sc_pak_header_len);
                char *pRecTtBuf = (char*)pBuffer+sc_pak_header_len;
                int iErrID = this->XBase_ReadData(pRecTtBuf, body_len, body_len, 0);
                if(iErrID>0)
                {
                    NETLOG_ERROR("[net.异常..1]"<<LogSelf()<<FUN_FILE_LINE);
                    PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
                    CloseSocketOut("ReadMsgBodyException!");
                    return false;
                }
            }
            else
            {
                recv_buf->m_IsStopRec = true;
                return false;
            }
            return true;
        }

    }
}
#endif //XZhtTCPBase_h__
