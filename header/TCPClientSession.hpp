#ifndef __OMP_NET_TCPCLIENTSESSION_HEADER__
#define __OMP_NET_TCPCLIENTSESSION_HEADER__

#include "./TCPXSessionBase.hpp"
namespace peony {
	namespace net {
		class XNoUsingAaa
		{
		public:
			XNoUsingAaa(){}
			~XNoUsingAaa(){}
		};
		class TCPClientSession : public XTcpSocketBase< TCPClientSession >
		{
			enum EmConnectionState
			{
				CS_NONE,
				CS_RESOLVING,
				CS_CONNECTING,
				CS_CONNECTED
			};

			void handle_resolve_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator);
			void handle_connect_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator);
			void post_connection_handler_zhtcc(unsigned conn_index,EmConnectInfo info,const std::string & msg);

		public:
			TCPClientSession( 
							unsigned         conn_index,
							TNewClientParam &param,
							TXZTBaseInitPm  &initPM,
							io_context      &io_service,
							AsioStrand&     strand );
			~TCPClientSession();

		public: //INiceNetSocket 接口部分
			virtual void	    StartSocket();

		public:
			bool				Run(unsigned uCount);
			int					SendMsg(const void* data_ptr, unsigned data_size);
			bool                CheckHeartTiimeout();

		protected: //接口部分
			virtual bool		XxTcpBeginRecWork();
			virtual void		XBase_ReadCb( const ZBoostErrCode& error,size_t bytes_transferred,int iDataType );
			virtual void		XBase_WriteCb(const ZBoostErrCode& error,int iDataType);

		private:
			void				read_packet_header();
			bool				DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len){ return false; }
			//因为多一个参数，所以包装一下
			void	            ImpPackIAppClientOnConnect( unsigned uMyServerID, unsigned uConnectID,int status, string info);

		protected:
			tcp::resolver		m_resolver;    //这个对象会引起创建线程
			AsioStrand		   &m_strand;
			runmsg_handler_type	m_runmsg_handler;
			EmConnectionState	m_EmConnectionState;
		public:
			TNewClientParam		m_Param;
		};
	}
}
#endif//#define __OMP_NET_TCPCLIENTSESSION_HEADER__