#ifndef WSClientSession_h__
#define WSClientSession_h__
#include "./WSSocketBase.hpp"

namespace peony {
	namespace net {
		class WSClientSession : public CWSSocketBase
		{
		public:
			enum EmConnectionState
			{
				CS_NONE,
				CS_RESOLVING,
				CS_CONNECTING,
				CS_CONNECTED
			};

		public:
			WSClientSession( unsigned conn_index,
							 TNewClientParam &param,
							 TXZTBaseInitPm  &initPM,
							 class Scheduler *pShedule);
			~WSClientSession();

		public:
			virtual void StartSocket();
			virtual bool OnRecive(unsigned uConnID, const void *pData, size_t data_len);

		protected:
			virtual bool XxTcpBeginRecWork();
			virtual int  HandRead_LogicConnect(size_t bytes_transferred);

		public:
			bool CheckHeartTiimeout();

		private:
			void post_connection_handler_zhtcc(unsigned conn_index, EmConnectInfo info, const std::string &msg);
			void ImpPackIAppClientOnConnect(unsigned uMyServerID, unsigned uConnectID, int status, string info);
			void handle_resolve_zhtcc(const ZBoostErrCode& err, tcp::resolver::iterator endpoint_iterator);
			void handle_connect_zhtcc(const ZBoostErrCode& err, tcp::resolver::iterator endpoint_iterator);

			void SendHandshakeMsgReq();
		private:
			class Scheduler *   m_pSch;
			tcp::resolver       m_Resolver;

			TNewClientParam		m_Param;
			EmConnectionState	m_EmConnectionState;
			runmsg_handler_type	m_runmsg_handler;

		};
	}
}
#endif//#define __OMP_NET_TCPCLIENTSESSION_HEADER__