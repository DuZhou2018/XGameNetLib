/*****************************************************************************************************************************
    Created:   2017/9/6
    Author:    ZhangHongtao,   1265008170@qq.com
    FileName:  G:\QzhXGame\PeonyNetWebSocket\header\HttpServerSession.hpp
 
******************************************************************************************************************************/
#ifndef __OMP_TCPCONNECTION_HEADER__HTTP
#define __OMP_TCPCONNECTION_HEADER__HTTP
#include "./NiceHttpReq.h"
#include "./TCPXSessionBase.hpp"

namespace peony {
	namespace net {		

		class HttpServerSession : public XTcpSocketBase< HttpServerSession >
		{
		public:
			HttpServerSession(  io_context             &io_service,
								TXZTBaseInitPm          &initPM,
								const TNewServerParam   &param);
			~HttpServerSession();

		public: //INiceNetSocket 接口部分
			virtual void	StartSocket();

		public:
			int 			SendMsg( const void* data_ptr, size_t data_size );
			bool			Run(unsigned uCount);
			IHttpReq *		GetHttpReq(){ return &m_OneReq; }

		protected:
			virtual bool    XxTcpBeginRecWork();
			virtual bool	DoNetInCmd(unsigned connectid, IHttpReq *pHttpReq );
			virtual void	XBase_ReadCb( const ZBoostErrCode& error,size_t bytes_transferred,int iDataType );
			virtual void	XBase_WriteCb(const ZBoostErrCode& error,int iDataType);


		private:
			bool			read_OneHttpReq(unsigned uReadLen=0);

		private:
			CNHttpReq       m_OneReq;

		public:
			const TNewServerParam &m_param;
		};		

	}	// end of namespace net

}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
