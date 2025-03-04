#ifndef WebSocketSession_h__
#define WebSocketSession_h__
#include "./WSSocketBase.hpp"

namespace peony {
    namespace net {
        class WebSocketSession : public CWSSocketBase
        {
        public:
            WebSocketSession( io_context &io_service,TXZTBaseInitPm &initPM, const TNewServerParam &param);
			virtual ~WebSocketSession();

		public:
			virtual void StartSocket();
			virtual bool OnRecive(unsigned uConnID, const void *pData, size_t data_len);

		protected:
			virtual bool XxTcpBeginRecWork();
			virtual int  HandRead_LogicConnect(size_t bytes_transferred);

		protected:
			/**********************************************************************************
				Created:   2017/9/7 	Author:    ZhangHongtao,   1265008170@qq.com
				下面的分段是逻辑链接处理部分
			********************************************************************************/
			void            HandRead_LogicConnect_DoShaKey(string strSecWebSocketKey);
			int 			HandRead_LogicConnect_SendShaKeyRep();

		public:
			const TNewServerParam  &m_param;

        };

    }	// end of namespace net
}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
