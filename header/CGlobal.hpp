
#ifndef _PY_NET_GLOBAL_OBJ
#define _PY_NET_GLOBAL_OBJ
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include "../header/NiceNetLogicProtocol.h"


namespace peony {
	namespace net {

		enum emZhtControl
		{
			EmZhtCon_ClearAll            = 1<<0,     //清楚所有     "EmZhtCon_ClearAll"
			EmZhtCon_NotRunMsg           = 1<<1,     //不处理消息   "EmZhtCon_NotRunMsg"
			EmZhtCon_NotAllcocNewConnect = 1<<2,     //不接收新链接 "EmZhtCon_NotAllcocNewConnect"

		};

		typedef boost::function< void(unsigned uGrade,string &strkind,stringstream &strIn,bool isnetlog)> handle_netlog_interface;
		class PNGB
		{
        public:
			static class CClientServerIDMG		*m_pCSIDMG;
			static class TCPClientMg			*m_pTCPClient;
			static class WSClientMg			    *m_pWSClient;
			static class UDPClientMg			*m_pUDPClient;
            static class Scheduler				*m_pScheduler;
			static class CNiceIDMG				*m_pNiceMsgMG;
			static class CCountersMg			*m_pCountersMg;
			static class INiceLog				*m_pLog;
			static class CNetCounterInstanceMg  *m_pNetInCounterMg;
			static boost::recursive_mutex	    *m_globMutex;
			static class CAllocMg               *m_pAllocMg;
			static class CNetLogForOpc          *m_pNetLogOpcTool;
            static TNiceNetExFunFace            *m_NetExFunMg;         //外包函数接口 
            static bool                          m_isbegin_closesys;	//是否开始关闭系统
			static unsigned                      m_zhtcontroler;		//控制器
			static unsigned                      m_zhtc_slowertime;
			static unsigned                      m_server_relativeTm;	//百分之一秒的精度
			static unsigned                      m_server_curTm;	    //秒的精度
			static bool                          m_IsCheckPackHead;     //是否校验包头

        public:
			static handle_netlog_interface       m_interfun_netlog;

        public:
            static void CreateGlobalObj();
            static void DeleteGlobalObj();
			static void NetMsg_ZhtControler(unsigned connectid,const peony::net::INPROTOCOL::NiceNetPZhtControlLogicReq *pReq);
			static bool IsXMarked( unsigned mark ){ return (m_zhtcontroler&mark)>0; }
		public:
			PNGB();
		};
	}	// end of namespace net

}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
