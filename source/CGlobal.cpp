 #include "../header/CGlobal.hpp"

#include "../header/InPublicPreDefine.hpp"
#include "../header/NiceIDMg.h"
#include "../header/CAllocMg.h"
#include "../header/NetLogMg.h"
#include "../header/XXClientMg.hpp"
#include "../header/XXNetServer.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CountersMg.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/NetCountersInstanceMg.h"

#ifdef WIN32
#ifdef STLPORT
#pragma message("you using stlport............")
#else
#pragma message("you using win stl............")
#endif
#endif

namespace peony
{
    namespace net
    {	
        INiceLog				* PNGB::m_pLog       		 =0;
        CCountersMg				* PNGB::m_pCountersMg		 =0;
        CClientServerIDMG		* PNGB::m_pCSIDMG    		 =0;
        Scheduler				* PNGB::m_pScheduler 		 =0;
        TCPClientMg				* PNGB::m_pTCPClient 		 =0;
		WSClientMg			    * PNGB::m_pWSClient			 =0;
		UDPClientMg			    * PNGB::m_pUDPClient		 =0;
        CNiceIDMG				* PNGB::m_pNiceMsgMG 		 =0;
		CNetCounterInstanceMg	* PNGB::m_pNetInCounterMg	 =0;
		CAllocMg                * PNGB::m_pAllocMg           =0;
		boost::recursive_mutex	* PNGB::m_globMutex          =0;
		CNetLogForOpc           * PNGB::m_pNetLogOpcTool     =0;
        TNiceNetExFunFace       * PNGB::m_NetExFunMg         =0;
        bool                      PNGB::m_isbegin_closesys   =false;
		unsigned                  PNGB::m_zhtcontroler       =0;
		unsigned                  PNGB::m_zhtc_slowertime    =1576850526;//20191220T220206
		unsigned                  PNGB::m_server_relativeTm  =0;
		unsigned                  PNGB::m_server_curTm       =0;
		handle_netlog_interface   PNGB::m_interfun_netlog    =0;
		bool                      PNGB::m_IsCheckPackHead    =true;
		bool                            g_is_random_ltmsg    =false;
		
        void PNGB::CreateGlobalObj()
        {
            //PEYONE_LOG_NORMAL(m_pLog,"Begin peyone net ...............");
            //assert(m_pLog);

			m_globMutex         = new boost::recursive_mutex();
			m_pAllocMg          = new CAllocMg();
            m_pCountersMg		= new CCountersMg();
            m_pCSIDMG			= new CClientServerIDMG();
            m_pNiceMsgMG		= new CNiceIDMG();
            m_pScheduler		= new Scheduler();
            m_pTCPClient		= new TCPClientMg(m_pScheduler);
			m_pWSClient         = new WSClientMg(m_pScheduler);
            m_pUDPClient        = new UDPClientMg(m_pScheduler);
			m_pNetInCounterMg	= new CNetCounterInstanceMg();
			m_pNetLogOpcTool    = new CNetLogForOpc();
			m_zhtcontroler      = 0;
			m_server_relativeTm = 0;

			m_interfun_netlog   = boost::bind( &CNetLogForOpc::AddLog,m_pNetLogOpcTool,_1,_2,_3,_4 );
        }
        void PNGB::DeleteGlobalObj()
        {
			m_interfun_netlog = 0;
            DELETE_OBJ(m_pTCPClient)
            DELETE_OBJ(m_pUDPClient)
            DELETE_OBJ(m_pScheduler)
            DELETE_OBJ(m_pNiceMsgMG)
            DELETE_OBJ(m_pCSIDMG)
			DELETE_OBJ(m_pNetInCounterMg)
            DELETE_OBJ(m_pCountersMg)
			DELETE_OBJ(m_pAllocMg)
            DELETE_OBJ(m_pLog)
			DELETE_OBJ(m_pNetLogOpcTool)
			DELETE_OBJ(m_globMutex)
        }

		PNGB::PNGB()
		{
			PNGB::CreateGlobalObj();
		}

		void PNGB::NetMsg_ZhtControler( unsigned connectid,const peony::net::INPROTOCOL::NiceNetPZhtControlLogicReq *pReq )
		{
			INPROTOCOL::NiceNetPZhtControlLogicRep repMsg;
			repMsg.marked = pReq->marked;
			repMsg.result = 0;

			if( "EmZhtCon_ClearAll"==string(pReq->zhtcontroltype) )
			{
				PNGB::m_zhtcontroler = 0;
			}
			else if( "EmZhtCon_NotAllcocNewConnect"==string(pReq->zhtcontroltype) )
			{
				PNGB::m_zhtcontroler |= EmZhtCon_NotAllcocNewConnect;
			}
			else if( "EmZhtCon_NotRunMsg"==string(pReq->zhtcontroltype) )
			{
				PNGB::m_zhtcontroler |= EmZhtCon_NotRunMsg;
			}
			else if( "EmZhtCon_OpenLostMsg"==string(pReq->zhtcontroltype) )
			{
				g_is_random_ltmsg = true;
			}
			else if( "EmZhtCon_CloseLostMsg"==string(pReq->zhtcontroltype) )
			{
				g_is_random_ltmsg = false;
			}
			else
			{
				repMsg.result = 10;
			}
			INiceNet::Instance().SendMsg( connectid,&repMsg,repMsg.GetLen() );
		}
		//static PNGB g_pngb;
	}	// end of namespace net

}	// end of namespace peony

