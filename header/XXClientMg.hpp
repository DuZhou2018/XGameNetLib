#ifndef __TCPCLIENT_IMPL_HEADER__
#define __TCPCLIENT_IMPL_HEADER__
#include "./XClientMgBase.h"
#include "../header/TCPClientSession.hpp"
#include "../header/UDPClientSession.hpp"
#include "../header/WSClientSession.hpp"

namespace peony{
	namespace net{		
		class TCPClientMg : public XClientMgBase<TCPClientSession>
		{
		public:
			TCPClientMg(class Scheduler* pScheduler):XClientMgBase(pScheduler){}
			~TCPClientMg(){}

		private:
			TCPClientSessionPtr	create_session(TNewClientParam &param);

		};

		class UDPClientMg : public XClientMgBase<UDPClientSession>
		{
		public:
			UDPClientMg(class Scheduler* pScheduler) :XClientMgBase(pScheduler) {}
			~UDPClientMg() {}

		private:
			UDPClientSessionPtr	create_session(TNewClientParam &param);
		};

		class WSClientMg : public XClientMgBase<WSClientSession>
		{
		public:
			WSClientMg(class Scheduler* pScheduler) :XClientMgBase(pScheduler) {}
			~WSClientMg() {}

		public:
			int	SendWebSocketMsg(unsigned conn_index, const void *pdata, unsigned data_len, bool IsText);

		private:
			WSClientSessionPtr	create_session(TNewClientParam &param);
		};
	}
}
#endif//#define __TCPCLIENT_IMPL_HEADER__