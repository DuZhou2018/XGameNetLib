/*****************************************************************************************************************************
Created:   2017/9/12
Author:    ZhangHongtao,   1265008170@qq.com
FileName:  G:\QzhXGame\PeonyNet\header\NetXServerBase.h

******************************************************************************************************************************/
#ifndef NetXServerBase_h__
#define NetXServerBase_h__
#include "./INiceNetServer.h"
#include "./CGlobal.hpp"
#include "./ClientServerIDMG.h"
#include "./CountersMg.hpp"
#include "./NiceIDMg.h"

namespace peony
{
	namespace net
	{
		template < class XServer,class XSession >
		class NetXServerBase : public INiceNetServer
		{
			typedef boost::shared_ptr<XSession>  ZAXSessionPtr;
			//typedef map_a<unsigned,XSession*>  XMapSess;
			typedef map<unsigned,XSession*>  XMapSess;

		protected:
			NetXServerBase( TNewServerParam &param,server_deallocator_type DelSelfFun );
		public:
			virtual ~NetXServerBase();


		public:
			virtual bool		 start( void );
			virtual unsigned get_conn_count( void ){ return (unsigned)(m_ConnMap_byIndex.size() - 1);}
			virtual void		 stop( bool wait_until_finished /*= true */ );
			virtual string       GetListenIpPort(){ return boost::str( boost::format("%s:%u")%m_param.strServerIp%m_param.uServerPort ); }
			virtual unsigned GetMaxCounterCount(){ return m_param.uOpenCounterCount; }

			virtual string       GetRemoteIp(unsigned &uPort,unsigned conn_index);
			virtual string       GetLocalIp(unsigned &uPort,unsigned conn_index);
			virtual bool         Server_GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info);
			virtual bool         Server_ClearSeeionBufferInfo(unsigned uConnectid);
			virtual unsigned GetSeverCurConnectCount();

			virtual void         SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel );
			virtual bool         IsXAttrib(unsigned conid,unsigned xmark);

			virtual void         SetConData(unsigned conid,uint64_t udata);
			virtual uint64_t     GetConData(unsigned conid);
			virtual IAppServer  *GetIAppServer(){ return m_param.pApp; }
			virtual void         OnTimeCheckHeartTimeout(LIST_UInt &listConID);
			virtual void         CheckIsStopAccept(); 
			virtual INetSocketPtr create_session();
			virtual int 		 SendMsg(const unsigned &uConnectID,const void *pData,unsigned uLength){ return this->send( uConnectID,pData,uLength);}
			virtual int          GetCurSessionsPoolSize(){ return 0; }

			bool                 is_may_accept_newconnect(string strRemoteIp); //是否可以接收新连接

			int					 send( unsigned conn_index,const void *data_ptr,size_t data_len );
			bool				 CloseSub( unsigned conn_index,string strWhy );
			XSession *           GetXSessionByConID( unsigned uConID );

		protected:
			bool				 PNetListen();
			void				 handle_accept( INetSocketPtr session_ptr,const ZBoostErrCode& error );

			void                 destroy_session(XSession * session);

		protected:
			void				 callback_byapp(  XSession * session );

		public:
			TNewServerParam					m_param;
		protected:
			boost::function<void(XSession*)>m_session_deallocator;			
			server_deallocator_type			m_self_deallocator;
			XMapSess  						m_ConnMap_byIndex;
			XServerSPtr						m_pSelfAutoPoint;

		};

		template < class XServer,class XSession >
		XSession * peony::net::NetXServerBase<XServer, XSession>::GetXSessionByConID(unsigned uConID)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConID );
			if( it!=m_ConnMap_byIndex.end() )
			{
				return it->second;
			}
			return 0;
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::start(void)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			// NOT allow the acceptor to reuse the address (i.e. SO_REUSEADDR)
			m_acceptor.open(m_endpoint.protocol());
			//设置地址不可以重用。
			//m_acceptor.set_option(tcp::acceptor::reuse_address(false) );
			//设置地址可以重用
			m_acceptor.set_option(tcp::acceptor::reuse_address(true) );

			ZBoostErrCode	error;
			//	bind endpoint
			m_acceptor.bind(m_endpoint,error);
			if(error)
			{
				NETLOG__ERROR("bind ip,port fail! "<<error.message()<<FUN_FILE_LINE);
				return false;
			}

			//	start listen
			m_acceptor.listen(boost::asio::socket_base::max_connections,error);
			if(error)
			{
				NETLOG__ERROR("!! "<<error.message()<<FUN_FILE_LINE);
				return false;
			}

			PNetListen();

			//生成计数器信息
			//PNGB::m_pNetInCounterMg->NoticeNewConnect( this->GetID(),CS_ADD_SERVER );
			return true;
		}

		template < class XServer,class XSession >
		int peony::net::NetXServerBase<XServer, XSession>::send(unsigned conn_index,const void *data_ptr,size_t data_len)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			std::string data(reinterpret_cast<const char *>(data_ptr),data_len);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				// maybe the client has been closed before this
				//OMP_LOG_DEBUG(PNGB::m_pScheduler->get_logger(),"TCPServer::send() invalid conn_index: " << conn_index
				//	<< " file: " << __FILE__ << " line: " << __LINE__ );
				return 952101;
			}
			else
			{
				XSession* pSession = it->second;
				return pSession->SendMsg( data.c_str(),(unsigned)data.length() );
			}
			return 0;
		}
		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::CloseSub(unsigned conn_index,string strWhy)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				(it->second)->close_out( strWhy );
				return true;
			}
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::is_may_accept_newconnect(string strRemoteIp)
		{
			if( PNGB::IsXMarked(EmZhtCon_NotAllcocNewConnect) )
			{
				NETLOG__ERROR("[.......!!] EmZhtCon_NotAllcocNewConnect! m_instance_id="<<m_instance_id<<FUN_FILE_LINE);
				return false;
			}

			if( m_ConnMap_byIndex.size()>=m_param.uMaxConnectCount )
				return false;
			if( false == CIpLimitMg::Instance().IsAllocCon(strRemoteIp))
			{
				return false;
			}
			CIpLimitMg::Instance().AddOneIp( strRemoteIp );
			//统计数据
			this->StatOneNewConnect();
			return true;
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::handle_accept(INetSocketPtr session_ptr,const ZBoostErrCode& error)
		{
			//是成功的吗,true表示是
			bool isok = false;
			if( !error )
				isok = true;

			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(out_callback_in_handle_accept,this,session_ptr,isok) );
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(out_callback_begin_listen,this),true );
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::PNetListen()
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			INetSocketPtr smart_ptr = create_session();
			if( smart_ptr == NULL )
			{
				m_ListenCon.isStop  = true;
				m_ListenCon.uStopTm = PNGB::m_server_curTm;
				NETLOG__ERROR("[TcpServer.create_session_tolisten_fail] listen. stop .. fail;"<<FUN_LINE );
				return false;
			}

			tcp::socket &tMySocket = *(smart_ptr->GetMySocket());
			m_acceptor.async_accept( tMySocket,
				m_strand.wrap(boost::bind( &peony::net::NetXServerBase<XServer, XSession>::handle_accept,
				this,
				smart_ptr,
				boost::asio::placeholders::error ) ));

			return true;
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::CheckIsStopAccept()
		{
			if( false == m_ListenCon.isStop )
				return;
			if( PNGB::m_server_curTm-m_ListenCon.uStopTm<10 )
				return;
			NETLOG__ERROR("[ TcpFind this server stop accept ] strServerIp="<<m_param.strServerIp<<"; port="<<m_param.uServerPort<<FUN_LINE );

			m_ListenCon.isStop  = false;
			m_ListenCon.uStopTm = 0;
			this->PNetListen();
		}

		template < class XServer,class XSession >
		INetSocketPtr peony::net::NetXServerBase<XServer, XSession>::create_session()
		{
			//Boost_Scoped_Lock server_lock(m_mutex);

			TXZTBaseInitPm tInitPm;
			tInitPm.mXServerID	= m_instance_id;
			tInitPm.r_size		= m_param.uReciveBuffSize;
			tInitPm.s_size		= m_param.uSendBuffSize;
			tInitPm.pStatData	= &m_StatData;
			tInitPm.mFunConnect = boost::bind( &IAppServer::OnSubConnect,m_param.pApp,_1,_2,_3,_4);

			tInitPm.r_alloc		= boost::bind(tcpserver_in_alloc_buffer,false,_1,_2,_3,_4);
			tInitPm.s_alloc		= boost::bind(tcpserver_in_alloc_buffer,false,_1,_2,_3,_4);

			XSession* session_ptr= new XSession( m_io_service,tInitPm,m_param);
			if( session_ptr->IsAllocSendRecBuffer() )
			{
				session_ptr->GetNewConID();
				session_ptr->SetMyXServerPtr( XServerSPtr() );
			}else
			{
				NETLOG__WARNING("当前缓冲去已经使用完! m_instance_id="<<m_instance_id<<FUN_FILE_LINE);
				delete session_ptr;
				return INetSocketPtr();
			}

			session_ptr->SetMyXServerPtr( this->m_pSelfAutoPoint );
			INetSocketPtr smart_ptr = INetSocketPtr(session_ptr,m_session_deallocator);
			m_ConnMap_byIndex.insert( std::make_pair(session_ptr->get_conn_index(),session_ptr));
			return smart_ptr;
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::callback_byapp(XSession * session)
		{
			unsigned conn_index = session->get_conn_index();
			PNGB::m_pCountersMg->ServerConnectClose( conn_index );
			{
				//Boost_Scoped_Lock server_lock(m_mutex);
				//NETLOG__NORMAL("删除连接:Server:"<<m_instance_id<<";close "<<session->get_conn_index() );

				session->FreeConID();
				session->SetMyXServerPtr( XServerSPtr() );
				m_ConnMap_byIndex.erase(conn_index);

				delete session;
				session=0;
			}
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::destroy_session(XSession * session)
		{
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(&peony::net::NetXServerBase<XServer, XSession>::callback_byapp,this,session) );
		}

		template < class XServer,class XSession >
		peony::net::NetXServerBase<XServer, XSession>::NetXServerBase(TNewServerParam &param,server_deallocator_type DelSelfFun)
		{
			m_session_deallocator=m_strand.wrap(boost::bind(&peony::net::NetXServerBase<XServer, XSession>::destroy_session,this,_1));
			m_self_deallocator=DelSelfFun;
			m_param = param;


			m_pSelfAutoPoint = XServerSPtr(this,m_self_deallocator );
			m_instance_id    = PNGB::m_pCSIDMG->GetNewID(CClientServerIDMG::IDKIND_SERVER,0);
			assert(m_instance_id);
			boost::asio::ip::address_v4	addr;
			if(0==strlen(param.strServerIp))
			{
				addr = boost::asio::ip::address_v4::from_string("0,0.0.0");
				m_endpoint=tcp::endpoint(addr,param.uServerPort); //不限制内外网限制Ip
			}
			else
			{
				addr = boost::asio::ip::address_v4::from_string(param.strServerIp);
				m_endpoint=tcp::endpoint( addr,param.uServerPort ); //限制Ip
			}
		}

		template < class XServer,class XSession >
		peony::net::NetXServerBase<XServer, XSession>::~NetXServerBase()
		{

		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::stop(bool wait_until_finished /*= true */)
		{
			m_ListenCon.uDelBeginTm = PNGB::m_server_curTm;
			m_pSelfAutoPoint = XServerSPtr();
			/*if this fuction is called in main thread,the call_close_handler may not acctual
			executed the session's close_handler ,instead of posted by service by strand.
			So we only do some clear action at first  and then wait.
			*/
			if( true )
			{
				//Boost_Scoped_Lock server_lock(m_mutex);
				ZBoostErrCode ec;
				m_acceptor.close(ec);
				if (ec)
				{
					string streeeor = ec.message();
					NETLOG__ERROR(ec.message()<<FUN_FILE_LINE);
				}
				//m_acceptor.close();
			}

			if( m_ConnMap_byIndex.size()>0 )
			{
				for(typename XMapSess::iterator it=m_ConnMap_byIndex.begin();it!=m_ConnMap_byIndex.end();)
				{
					XSession * session = it->second;
					session->close_out("stopapp!");
					++it;
				}
			}
		}

		template < class XServer,class XSession >
		string peony::net::NetXServerBase<XServer, XSession>::GetRemoteIp(unsigned &uPort,unsigned conn_index)
		{
			uPort = 0;
			std::string ret="";
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				// maybe the client has been closed before this
				//OMP_LOG_DEBUG(PNGB::m_pScheduler->get_logger(),"TCPServer::get_ip_addr() invalid conn_index: " << conn_index
				//	<< " file: " << __FILE__ << " line: " << __LINE__ );
			}else
			{
				XSession* pSession = it->second;
				try{
					ret = pSession->GetRemoteIp( uPort );
				}catch(ZBoostErrCode& error)
				{
					NETLOG__ERROR("exception!! connecid:"<<conn_index<<";"<<error.message()<<FUN_FILE_LINE);
				}
			}
			return ret;
		}

		template < class XServer,class XSession >
		string peony::net::NetXServerBase<XServer, XSession>::GetLocalIp(unsigned &uPort,unsigned conn_index)
		{
			uPort = 0;
			std::string ret="";
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conn_index );
			if( it==m_ConnMap_byIndex.end() )
			{
				// maybe the client has been closed before this
				//OMP_LOG_DEBUG(PNGB::m_pScheduler->get_logger(),"TCPServer::get_ip_addr() invalid conn_index: " << conn_index
				//	<< " file: " << __FILE__ << " line: " << __LINE__ );
			}else{
				XSession* pSession = it->second;
				try{
					ret = pSession->GetLocalIp( uPort );
				}catch(ZBoostErrCode& error)
				{
					NETLOG__ERROR("exception!! connecid:"<<conn_index<<";"<<error.message()<<FUN_FILE_LINE);
				}
			}
			return ret;
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::Server_GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConnectid );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				XSession* pSession = it->second;
				pSession->GetSessionBufferInfo( info );
			}
			return true;
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer, XSession>::Server_ClearSeeionBufferInfo(unsigned uConnectid)
		{
			//Boost_Scoped_Lock server_lock(m_mutex);

			typename XMapSess::iterator it = m_ConnMap_byIndex.find( uConnectid );
			if( it==m_ConnMap_byIndex.end() )
			{
				return false;
			}else
			{
				XSession* pSession = it->second;
				pSession->ClearSeeionBufferInfo();
			}
			return true;
		}

		template < class XServer,class XSession >
		unsigned peony::net::NetXServerBase<XServer, XSession>::GetSeverCurConnectCount()
		{
			//Boost_Scoped_Lock server_lock(m_mutex);
			return (unsigned)(m_ConnMap_byIndex.size()-1);
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::SetXAttrib(unsigned conid,unsigned xadd, unsigned xdel)
		{
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return;
			XSession* pSession = it->second;
			pSession->SetXMark(xadd,xdel);
		}

		template < class XServer,class XSession >
		bool peony::net::NetXServerBase<XServer,XSession>::IsXAttrib(unsigned conid,unsigned xmark)
		{
			typename XMapSess::iterator it = m_ConnMap_byIndex.find(conid);
			if(it==m_ConnMap_byIndex.end())
				return false;
			XSession* pSession = it->second;
			return pSession->IsXMark(xmark);
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::SetConData(unsigned conid,uint64_t udata)
		{
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return;
			XSession* pSession = it->second;
			pSession->SetConData(udata);
		}

		template < class XServer,class XSession >
		uint64_t peony::net::NetXServerBase<XServer, XSession>::GetConData(unsigned conid)
		{
			typename XMapSess::iterator it = m_ConnMap_byIndex.find( conid );
			if( it==m_ConnMap_byIndex.end() )
				return 0;
			XSession* pSession = (XSession*)(it->second);
			return pSession->GetConData();
		}

		template < class XServer,class XSession >
		void peony::net::NetXServerBase<XServer, XSession>::OnTimeCheckHeartTimeout(LIST_UInt &listConID)
		{
			typename XMapSess::iterator it = m_ConnMap_byIndex.begin();
			for( ;it!=m_ConnMap_byIndex.end(); it++ )
			{
				if( it->second->is_heart_timeout(m_param.uHeartTmSecond) )
				{
					NETLOG__ERROR("TCPServer..TimeOut: "<<FUN_LINE );
					listConID.push_back( it->first );
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
	}
}
#endif // NetXServerBase_h__
