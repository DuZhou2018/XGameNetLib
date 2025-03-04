/***************************************************************************************************************************************
	author:	    DuZhou   qq: 1265008170,   mail: zht213@163.com
	created:    2021/9/28 14:06
	filename:   UdpServerBase.h
	purpose:	
***************************************************************************************************************************************/
#ifndef NetUdpXServerBase_h__
#define NetUdpXServerBase_h__
#include "./INiceNetServer.h"
#include "./CGlobal.hpp"
#include "./ClientServerIDMG.h"
#include "./CountersMg.hpp"
#include "./NiceIDMg.h"

namespace peony
{
	namespace net
	{
        typedef map<udp::endpoint, unsigned>MapUdpPoint;
        typedef map<unsigned, udp::endpoint>MapAaaUdpPoint;

		template < class XSession >
		class UdpServerBase : public INiceNetServer
		{
			typedef map<unsigned, XSession*>  XMapSess;

		protected:
			UdpServerBase( TNewServerParam &param,server_deallocator_type DelSelfFun );
		public:
			virtual ~UdpServerBase();
            
		public:
			virtual bool		 StartSocket( void );
			virtual unsigned     get_conn_count( void ){ return (unsigned)(m_ConnMap_byIndex.size() - 1);}
			virtual void		 stop( bool wait_until_finished /*= true */ );
			virtual string       GetListenIpPort(){ return boost::str( boost::format("%s:%u")%m_param.strServerIp%m_param.uServerPort ); }
			virtual unsigned     GetMaxCounterCount(){ return m_param.uOpenCounterCount; }

			virtual string       GetRemoteIp(unsigned &uPort,unsigned conn_index);
			virtual string       GetLocalIp(unsigned &uPort,unsigned conn_index);
			virtual bool         Server_GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info);
			virtual bool         Server_ClearSeeionBufferInfo(unsigned uConnectid);
			virtual unsigned     GetSeverCurConnectCount();

			virtual void         SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel );
			virtual bool         IsXAttrib(unsigned conid,unsigned xmark);

			virtual void         SetConData(unsigned conid,uint64_t udata);
			virtual uint64_t     GetConData(unsigned conid);
			virtual IAppServer  *GetIAppServer(){ return m_param.pApp; }
			virtual void         OnTimeCheckHeartTimeout(LIST_UInt &listConID);
			virtual void         CheckIsStopAccept(); 
			virtual int 		 SendMsg(const unsigned &uConnectID,const void *pData,unsigned uLength){ return this->send( uConnectID,pData,uLength);}
			virtual int          GetCurSessionsPoolSize(){ return 0; }
            virtual void		 SendOne_WriteDataCb(const ZBoostErrCode& error, std::size_t iDataType);

			int                  is_may_accept_newconnect(string strRemoteIp); //是否可以接收新连接
			int					 send( unsigned conn_index,const void *data_ptr,size_t data_len );
			bool				 CloseSub( unsigned conn_index,string strWhy );
			XSession *           GetXSessionByConID( unsigned uConID );

		protected:
			INetSocketPtr		 create_session(udp::endpoint &tEndpoint);
			bool				 PNetListen() { return true; };
			void                 destroy_session(XSession * session);
			void				 callback_byapp(  XSession * session );
            void                 handleRecvTime(const ZBoostErrCode& error,  std::size_t bytes_transferred);
            unsigned             GetSocketIndexByUdpPoint( udp::endpoint tPoint );
            void                 RunSendOneMsg();
            int                  SendOne_WriteData(const char *pBuffer, unsigned uMaxLen, udp::endpoint &desPoint );

		public:
			TNewServerParam					m_param;
		protected:
			boost::function<void(XSession*)>m_session_deallocator;			
			server_deallocator_type			m_self_deallocator;
            MapUdpPoint                     m_ConnMap_PtToIndex;
            MapAaaUdpPoint                  m_ConnMap_IndexToPt;
            XMapSess  						m_ConnMap_byIndex;
			XServerSPtr						m_pSelfAutoPoint;

            UdpSocket	                    m_socket;
            udp::endpoint	                m_new_client_point;

            char                            m_RecBuf[2048];
            boost::recursive_mutex	        m_mexx_mutex;
            SendBufferType			        m_send_buf;
        };

        template < class XSession >
        peony::net::UdpServerBase<XSession>::UdpServerBase(TNewServerParam &param, server_deallocator_type DelSelfFun)
            : m_socket(PNGB::m_pScheduler->get_impl()->get_ioservice(), udp::endpoint(udp::v4(), param.uServerPort)),
            m_send_buf(boost::bind(tcpserver_in_alloc_buffer, false, _1, _2, _3, _4), 10240)
        {
            m_session_deallocator = m_strand.wrap(boost::bind(&peony::net::UdpServerBase<XSession>::destroy_session, this, _1));
            m_self_deallocator    = DelSelfFun;
            m_param = param;
            
            m_pSelfAutoPoint = XServerSPtr(this, m_self_deallocator);
            m_instance_id    = PNGB::m_pCSIDMG->GetNewID(IDKIND_SERVER, 0);
            assert(m_instance_id);
        }

        template < class XSession >
        peony::net::UdpServerBase<XSession>::~UdpServerBase()
        {

        }

		template < class XSession >
		XSession * peony::net::UdpServerBase<XSession>::GetXSessionByConID(unsigned uConID)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConID );
			if( it!=m_ConnMap_byIndex.end() )
			{
				return it->second;
			}
			return 0;
		}

		template < class XSession >
		bool peony::net::UdpServerBase<XSession>::StartSocket(void)
		{
			//生成计数器信息
			//PNGB::m_pNetInCounterMg->NoticeNewConnect( this->GetID(),CS_ADD_SERVER );
            memset( m_RecBuf,0,sizeof(m_RecBuf) );
            m_socket.async_receive_from(
                boost::asio::buffer(m_RecBuf,1024),
                m_new_client_point,
                boost::bind (&UdpServerBase::handleRecvTime, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
			return true;
		}
        template < class XSession >
        void peony::net::UdpServerBase<XSession>::handleRecvTime(const ZBoostErrCode& error, std::size_t bytes_transferred)
        {
            if( error || error==boost::asio::error::message_size )
            {
                NETLOG_ERROR("[ udpServer 接收数据错误 ] errormsg="<<error.message()<<FUN_LINE);
                return;
            }
			NETLOG_SYSINFO("[Udp接收到消息] m_new_client_point=" << m_new_client_point<<",bytes_transferred="<<bytes_transferred);
            unsigned iSockIndex = GetSocketIndexByUdpPoint(m_new_client_point);
            if( 0==iSockIndex )
            {
                INetSocketPtr smart_ptr = create_session(m_new_client_point);
                if(smart_ptr == NULL)
                {
                    NETLOG_ERROR("[UdpServerBase.handleRecvTime] ...... "<<FUN_LINE);
                    return;
                }
                iSockIndex = smart_ptr->GetConnectIndex();
            }

			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find(iSockIndex);
            XSession* pSession = it->second;
            pSession->PushRecOneMsg(m_RecBuf, (unsigned)bytes_transferred );
			this->StartSocket();
        }
		template < class XSession >
		bool peony::net::UdpServerBase<XSession>::CloseSub(unsigned conn_index,string strWhy)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				(it->second)->CloseSocketOut( strWhy );
				return true;
			}
		}

		template < class XSession >
		int peony::net::UdpServerBase<XSession>::is_may_accept_newconnect(string strRemoteIp)
		{
			if( PNGB::IsXMarked(EmZhtCon_NotAllcocNewConnect) )
			{
				NETLOG_ERROR("[.......!!] EmZhtCon_NotAllcocNewConnect! m_instance_id="<<m_instance_id<<FUN_LINE);
				return 878701;
			}

			if( m_ConnMap_byIndex.size()>=m_param.uMaxConnectCount )
				return 878703;
			if( false == CIpLimitMg::Instance().IsAllocCon(strRemoteIp))
			{
				return 878705;
			}
			CIpLimitMg::Instance().AddOneIp( strRemoteIp );
			//统计数据
			this->StatOneNewConnect();
			return 0;
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::CheckIsStopAccept()
		{
			if( false == m_ListenCon.isStop )
				return;
			if( PNGB::m_server_curTm-m_ListenCon.uStopTm<10 )
				return;
			NETLOG_ERROR("[ TcpFind this server stop accept ] strServerIp="<<m_param.strServerIp<<"; port="<<m_param.uServerPort<<FUN_LINE );

			m_ListenCon.isStop  = false;
			m_ListenCon.uStopTm = 0;
			this->PNetListen();
		}

		template < class XSession >
		INetSocketPtr peony::net::UdpServerBase<XSession>::create_session( udp::endpoint &tEndpoint )
		{
			TXZTBaseInitPm tInitPm;
			tInitPm.mXServerID	= m_instance_id;
			tInitPm.r_size		= m_param.uReciveBuffSize;
			tInitPm.s_size		= m_param.uSendBuffSize;
			tInitPm.pStatData	= &m_StatData;
			tInitPm.mFunConnect = boost::bind( &IAppServer::OnSubConnect,m_param.pApp,_1,_2,_3,_4);

			tInitPm.r_alloc		= boost::bind(tcpserver_in_alloc_buffer,false,_1,_2,_3,_4);
			tInitPm.s_alloc		= boost::bind(tcpserver_in_alloc_buffer,false,_1,_2,_3,_4);

			XSession* session_ptr = new XSession( m_io_service,tInitPm,m_param);
			if( true )
			{
				session_ptr->GetNewConID();
				session_ptr->SetMyXServerPtr( XServerSPtr() );
			}else
			{
				NETLOG_WARNING("当前缓冲去已经使用完! m_instance_id="<<m_instance_id<<FUN_LINE);
				delete session_ptr;
				return INetSocketPtr();
			}
            session_ptr->m_mepoint = tEndpoint;
            unsigned iConnIndex = session_ptr->GetConnectIndex();
			session_ptr->SetMyXServerPtr( this->m_pSelfAutoPoint );
			INetSocketPtr smart_ptr = INetSocketPtr(session_ptr,m_session_deallocator);
            session_ptr->NtfAppConnect( smart_ptr);
            if(1)
            {
                Boost_Scoped_Lock sendbuf_lock(m_mexx_mutex);
                m_ConnMap_IndexToPt[iConnIndex] = tEndpoint;
				m_ConnMap_byIndex[iConnIndex]   = session_ptr;
				m_ConnMap_PtToIndex[tEndpoint]  = iConnIndex;
			}
            return smart_ptr;
		}
        template < class XSession >
        unsigned peony::net::UdpServerBase<XSession>::GetSocketIndexByUdpPoint(udp::endpoint tPoint)
        {
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			MapUdpPoint::iterator it = m_ConnMap_PtToIndex.find(tPoint);
            if( it == m_ConnMap_PtToIndex.end() )
                return 0;
            return it->second;
        }

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::callback_byapp(XSession * session)
		{
			unsigned conn_index = session->GetConnectIndex();
			PNGB::m_pCountersMg->ServerConnectClose( conn_index );
			session->FreeConID();
			session->SetMyXServerPtr(XServerSPtr());

			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			m_ConnMap_byIndex.erase(conn_index);
            m_ConnMap_PtToIndex.erase(session->m_mepoint);
            m_ConnMap_IndexToPt.erase(conn_index);
			delete session;
			session=0;
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::destroy_session(XSession * session)
		{
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(&peony::net::UdpServerBase<XSession>::callback_byapp,this,session) );
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::stop(bool wait_until_finished /*= true */)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			m_ListenCon.uDelBeginTm = PNGB::m_server_curTm;
			m_pSelfAutoPoint = XServerSPtr();
            if( m_ConnMap_byIndex.size()>0 )
			{
				for(typename XMapSess::iterator it=m_ConnMap_byIndex.begin();it!=m_ConnMap_byIndex.end();)
				{
					XSession * session = it->second;
					session->CloseSocketOut("stopapp!");
					++it;
				}
			}
		}

		template < class XSession >
		string peony::net::UdpServerBase<XSession>::GetRemoteIp(unsigned &uPort,unsigned conn_index)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);

			uPort = 0;
			std::string ret="";
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				NETLOG_ERROR("[这个id不存在...] connecid:" << conn_index  << FUN_LINE);
			}else
			{
				XSession* pSession = it->second;
				try{
					ret = pSession->GetRemoteIp( uPort );
				}catch(ZBoostErrCode& error)
				{
					NETLOG_ERROR("exception!! connecid:"<<conn_index<<", exceptionInfo="<< error.message() <<FUN_LINE);
				}
			}
			return ret;
		}

		template < class XSession >
		string peony::net::UdpServerBase<XSession>::GetLocalIp(unsigned &uPort,unsigned conn_index)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);

			uPort = 0;
			std::string ret="";
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				NETLOG_ERROR("[这个id不存在...] connecid:" << conn_index << FUN_LINE );
			}else{ 
				XSession* pSession = it->second;
				try{
					ret = pSession->GetLocalIp( uPort );
				}catch(ZBoostErrCode& error)
				{
					NETLOG_ERROR("exception!! connecid:"<<conn_index<<"; exceptionInfo="<<error.message()<<FUN_LINE);
				}
			}
			return ret;
		}

		template < class XSession >
		bool peony::net::UdpServerBase<XSession>::Server_GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConnectid );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				//XSession* pSession = it->second;
				//pSession->GetSessionBufferInfo( info );
			}
			return true;
		}

		template < class XSession >
		bool peony::net::UdpServerBase<XSession>::Server_ClearSeeionBufferInfo(unsigned uConnectid)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);

			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConnectid );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				//XSession* pSession = it->second;
				//pSession->ClearSeeionBufferInfo();
			}
			return true;
		}

		template < class XSession >
		unsigned peony::net::UdpServerBase<XSession>::GetSeverCurConnectCount()
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			return (unsigned)(m_ConnMap_byIndex.size()-1);
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::SetXAttrib(unsigned conid,unsigned xadd, unsigned xdel)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return;
			XSession* pSession = it->second;
			pSession->SetXMark(xadd,xdel);
		}

		template < class XSession >
		bool peony::net::UdpServerBase<XSession>::IsXAttrib(unsigned conid,unsigned xmark)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find(conid);
			if(it==m_ConnMap_byIndex.end())
				return false;
			XSession* pSession = it->second;
			return pSession->IsXMark(xmark);
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::SetConData(unsigned conid,uint64_t udata)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return;
			XSession* pSession = it->second;
			pSession->SetConData(udata);
		}

		template < class XSession >
		uint64_t peony::net::UdpServerBase<XSession>::GetConData(unsigned conid)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return 0;
			XSession* pSession = (XSession*)(it->second);
			return pSession->GetConData();
		}

		template < class XSession >
		void peony::net::UdpServerBase<XSession>::OnTimeCheckHeartTimeout(LIST_UInt &listConID)
		{
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.begin();
            for( ;it!=m_ConnMap_byIndex.end(); it++ )
            {
             	if( it->second->CheckHeartTimeout(m_param.uHeartTmSecond) )
             	{
             		NETLOG_ERROR("UdpServer..TimeOut: ConnectIndex="<<it->first<<FUN_LINE );
             		listConID.push_back( it->first );
            	}
            }
		}

        template < class XSession >
        int peony::net::UdpServerBase<XSession>::send(unsigned conn_index, const void *data_ptr, size_t data_len)
        {
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			std::string data(reinterpret_cast<const char *>(data_ptr), data_len);
            typename XMapSess::iterator it = m_ConnMap_byIndex.find(conn_index);
            if(it==m_ConnMap_byIndex.end())
            {
                // maybe the client has been closed before this
                //OMP_LOG_DEBUG(PNGB::m_pScheduler->get_logger(),"TCPServer::send() invalid conn_index: " << conn_index
                //	<< " file: " << __FILE__ << " line: " << __LINE__ );
                return 952101;
            }
            else
            {
				tcp_pak_header *pHead = (tcp_pak_header*)data_ptr;
				pHead->SetUdpContextIndex(conn_index);
				m_send_buf.push(data_ptr, (unsigned)data_len);
				this->RunSendOneMsg();
			}
            return 0;
        }

        template < class XSession >
        void peony::net::UdpServerBase<XSession>::RunSendOneMsg()
        {
			Boost_Scoped_Lock recvbuf_lock(m_mexx_mutex);
			if (m_send_buf.IsEmpty())
                 return;

             unsigned uData_len = 0;
             char    *pSend		= (char*)m_send_buf.front(uData_len);

             bool ishappend_error= false;
             tcp_pak_header *ppak_header = (tcp_pak_header*)pSend;
             if(((unsigned)ppak_header->GetLen()) > m_send_buf.get_buffer_maxlen())
             {
                 //我日，包的长度比整个缓冲区还大，很明显包头错误
                 ishappend_error = true;
                 NETLOG_ERROR("[数据包头错误,将关闭连接]<6> 包头太长! pHeader->Len="<<ppak_header->GetLen()<<",conn_index="<<ppak_header->GetUdpContextIndex()<<FUN_LINE);
             }

             if(ppak_header->GetLen()<sc_pak_header_len)
             {
                 //我日，包的长度比包头还小，明显错误
                 ishappend_error = true;
                 NETLOG_ERROR("[数据包头错误,将关闭连接]<8> 包头太小[<12]! pHeader->Len="<<ppak_header->GetLen() );
             }
             unsigned iConIndex = ppak_header->GetUdpContextIndex();
             MapAaaUdpPoint::iterator ita = m_ConnMap_IndexToPt.find(iConIndex);
             if(ita == m_ConnMap_IndexToPt.end())
             {
                 m_send_buf.pop();
                 return;
             }
			 udp::endpoint &desPoint = ita->second;
             SendOne_WriteData( pSend,uData_len, desPoint);
        }

        template < class XSession >
        void peony::net::UdpServerBase<XSession>::SendOne_WriteDataCb(const ZBoostErrCode& error, std::size_t iDataType)
        {
            if( 1 )
            {
                Boost_Scoped_Lock sendbuf_lock(m_mexx_mutex);
                m_send_buf.pop();
            }
			if( error || error == boost::asio::error::message_size )
			{
				NETLOG_ERROR("[ udpServer 发送数据错误 ] errormsg=" << error.message() << FUN_LINE);
				return;
			}
			RunSendOneMsg();
        }

        template < class XSession >
        int peony::net::UdpServerBase<XSession>::SendOne_WriteData(const char *pBuffer, unsigned uMaxLen, udp::endpoint &desPoint )
        {
            try
            {
                m_socket.async_send_to(boost::asio::buffer(pBuffer, uMaxLen),
                    desPoint,
                    boost::bind (&INiceNetServer::SendOne_WriteDataCb,
                        boost::enable_shared_from_this<INiceNetServer>::shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
            }
            catch(...)
            {
                NETLOG_ERROR("[async_write异常]:"<<FUN_LINE);
                PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
                return 99908802;
            }
            return 0;
        }

		//////////////////////////////////////////////////////////////////////////
	}
}
#endif // NetXServerBase_h__
