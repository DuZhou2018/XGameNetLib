/***************************************************************************************************************************************
	author:	    DuZhou   qq: 1265008170,   mail: zht213@163.com
	created:    2021/9/28 15:33
	filename:   UDPClientSession.hpp
	purpose:	
***************************************************************************************************************************************/
#ifndef UDPClientSession_h__
#define UDPClientSession_h__

#include "./INiceNetServer.h"
namespace peony {
	namespace net {
		enum EmSocketMarketUdp
		{
			EMSocketUdp_BindPortFinish = 1<<21,  //已经完成了端口的绑定，可以接收数据
		};
		class UDPClientSession : public INiceNetSocket
		{			
			enum EmConnectionState
			{
				CS_NONE,
				CS_RESOLVING,
				CS_CONNECTING,
				CS_CONNECTED
			};

			void handle_udp_resolve_zhtcc(const ZBoostErrCode& err,udp::resolver::iterator endpoint_iterator);
			void post_connection_handler_zhtcc(unsigned conn_index,EmConnectInfo info,const std::string & msg);
			void handle_udp_connect_zhtcc(const ZBoostErrCode& err, udp::resolver::iterator endpoint_iterator);

		public:
			UDPClientSession( 
							unsigned         conn_index,
							TNewClientParam &param,
							TXZTBaseInitPm  &initPM,
							io_context      &io_service,
							AsioStrand		&strand );
			~UDPClientSession();

		public:
			virtual void	    StartSocket();

		public:
			bool				Run(unsigned uCount);
			int					SendMsg(const void* data_ptr, unsigned data_size);
			bool                CheckHeartTiimeout();
			void				GetSessionBufferInfo(TSessionBufferInfo &info){}
			void				ClearSeeionBufferInfo(){}

		private:
			bool				DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len){ return false; }
			void	            ImpPackIAppClientOnConnect( unsigned uMyServerID, unsigned uConnectID,int status, string info);
			
			int					XBase_ReadData();
			int					XBase_WriteData(const char *pBuffer, unsigned uMaxLen);
			void				XBase_UdpReadCb(const ZBoostErrCode& error, std::size_t bytes_transferred);
			void				XBase_UdpWriteCb(const ZBoostErrCode& error, std::size_t bytes_transferred);

		protected:
			EmConnectionState		m_EmConnectionState;
			runmsg_handler_type		m_runmsg_handler;

			UdpSocket               m_socket;
			udp::endpoint	        m_despoint;
			udp::endpoint	        m_serverpoint;
			udp::resolver			m_resolver;             //这个对象会引起创建线程
			AsioStrand			   &m_strand;

			// buffers
			boost::recursive_mutex	m_sendbuf_mutex;		//for m_send_buf's res. protection
			boost::recursive_mutex	m_recvbuf_mutex;		//for m_recv_buf's res. protection
			RecvBufferType			m_recv_buf;
			SendBufferType			m_send_buf;
		public:
			TNewClientParam			m_Param;
		};
		typedef boost::shared_ptr<UDPClientSession>UDPClientSessionPtr;
	}
}
#endif//#define __OMP_NET_TCPCLIENTSESSION_HEADER__