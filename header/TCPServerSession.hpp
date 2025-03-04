#ifndef __OMP_TCPCONNECTION_HEADER__
#define __OMP_TCPCONNECTION_HEADER__
#include "./TCPXSessionBase.hpp"

namespace peony {
	namespace net {		

		class TCPServerSession : public XTcpSocketBase< TCPServerSession >
		{
		public:			
			TCPServerSession(   io_context            &io_service,
								TXZTBaseInitPm         &initPM,
								const TNewServerParam   &param);
			~TCPServerSession();

		public: //INiceNetSocket 接口部分
			virtual void	StartSocket();

		public:
			int 			SendMsg( const void* data_ptr, unsigned data_size );
			bool			Run(unsigned uCount);

		protected: //接口部分
			virtual bool	XxTcpBeginRecWork();
			virtual void	XBase_ReadCb( const ZBoostErrCode& error,size_t bytes_transferred,int iDataType );
			virtual void	XBase_WriteCb(const ZBoostErrCode& error,int iDataType);

		private:
			virtual bool    DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len);
			void			read_packet_header();

		public:
			const TNewServerParam &m_param;
		};		

	}	// end of namespace net

}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
