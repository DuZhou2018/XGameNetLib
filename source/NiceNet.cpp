#include "../header/XXClientMg.hpp"
#include "../header/XXNetServer.hpp"
#include "../header/InPublicPreDefine.hpp"
#include "../header/NiceNet.h"
#include "../header/NiceLog.h"
#include "../header/NetLogMg.h"
#include "../header/CAllocMg.h"
#include "../header/NiceIDMg.h"
#include "../header/CGlobal.hpp"
#include "../header/Scheduler.hpp"
#include "../header/XXClientMg.hpp"
#include "../header/CountersMg.hpp"
#include "../header/NormalThreadMg.h"
#include "../header/NetCountersInstanceMg.h"

namespace peony
{
    namespace net
    {
        INiceNet & INiceNet::Instance()
        {
            return CNiceNet::Instance();
        }
		CNiceNet &CNiceNet::Instance()
		{
			static CNiceNet *pNicenet = new CNiceNet();
			return *pNicenet;
		}
		CNiceNet::CNiceNet(void)/*:m_mutex(*PNGB::m_globMutex)*/
        {
            m_IsInitOk			= false;
			m_NiceNetMark		= 0;
			m_LastCheckHeartTm	= 0;
        }

        CNiceNet::~CNiceNet(void)
        {
			NETLOG_SYSINFO(FUN_FILE_LINE);
			NETLOG_SAVELOG();
            if( m_IsInitOk )
            {
                UnInit();
            }
			PNGB::DeleteGlobalObj();
        }
        bool CNiceNet::Init( TNiceNetParam &param )
        {
			NETLOG_SYSINFO("Peony net begin init .....................................................................!");
			static PNGB g_pngb;
			PNGB::m_server_curTm = CInPubFun::GetNowTime();
            if( m_IsInitOk )
                return true;
			PNGB::m_IsCheckPackHead = param.IsCheckPackHead;
			PNGB::m_pLog = INiceLog::CreateLog( param.strLogFileName,param.strLogDir );
            if( !PNGB::m_pLog )
            {
                //std::cout<<__FUNCTION__";fail,"<<__LINE__<<__FILE__;
                return false;
            }

            PNGB::m_pLog->SetLogPutGrade(LOG_ALL_LOG);
			PNGB::m_pLog->SetNetLog();
			PNGB::m_pNetLogOpcTool->Init();
			CIpLimitMg::Instance().SetIpMaxCt( param.iSameIpMaxConnectCt );

			//把定时器移动到主循环里面
            //PNGB::m_pScheduler->add_timer(1000,boost::bind(&CNiceNet::OnTime_debug_serverthread,this) );
            assert( PNGB::m_pTCPClient );
            PNGB::m_pScheduler->startup( param.iWorkThreadNum );

            XGGTimerFunList tmFunList;
            if(param.IsOpenMThreadCheck) {
                tmFunList.push_back(TXRGQTimerFun(boost::bind(&CNiceNet::CheckAppMainThreadIsLock, this), 10121011, 3));
            }
            CPNetFunTaskMg::Single().Init(1, &tmFunList);

            m_IsInitOk = true;
            NETLOG_NORMAL("Peony net init sucess!");
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
            PNGB::m_pLog->SetLogPutGrade(param.iLogGrade);
            return true;
        }

        bool CNiceNet::InitExFunFace(TNiceNetExFunFace &param)
        {
            m_NetExFunFace = param;
            PNGB::m_NetExFunMg = &m_NetExFunFace;
            return true;
        }

        void CNiceNet::UnInit()
        {
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);

			if( !m_IsInitOk )
				return;
			PNGB::m_isbegin_closesys = true;
            //NETLOG_NORMAL("begin call..."<<FUN_FILE_LINE);

            m_IsInitOk = false;
            this->LogoutAllCounters();

			//停止底层的线程
			CPNetFunTaskMg::Single().StopWork();
			PNGB::m_pScheduler->shutdown();

			PNGB::m_pNiceMsgMG->UnInit();           
            if( PNGB::m_pTCPClient )
                PNGB::m_pTCPClient->UnInit();

            for(MAP_NETSERVER::iterator itor = m_mapNetServer.begin();itor!=m_mapNetServer.end();itor++)
            {
                INiceNetServer *pInServer = itor->second;
                pInServer->stop( true );
            }
            //PNGB::m_pNiceMsgMG->Run(0X0FFFFFFF,0);

            unsigned iServerSize = (unsigned)m_mapNetServer.size();
            unsigned iClientSize = PNGB::m_pTCPClient->GetSize();
            //NiceNet_lock.unlock();
			unsigned loopcur = 0;
            while( false )
            {
				loopcur++;
				if( loopcur>100*10 )
				{
					NETLOG_WARNING("UnInit waiting break...!"<<FUN_FILE_LINE);
					break;
				}

                if( 0==iServerSize && 0==iClientSize )
                    break;
                boost::this_thread::sleep( boost::posix_time::milliseconds(10) );
				do{
                    iServerSize = (unsigned)m_mapNetServer.size();
                }while(0);
                iClientSize = PNGB::m_pTCPClient->GetSize();
				unsigned uDoMsgCt= 0X0FFFFFFF;
                PNGB::m_pNiceMsgMG->Run(uDoMsgCt,0);

				if( iClientSize>0 )
				{
					if( PNGB::m_pTCPClient )
						PNGB::m_pTCPClient->UnInit();
				}
				if( iServerSize>0 )
				{
					for(MAP_NETSERVER::iterator itor = m_mapNetServer.begin();itor!=m_mapNetServer.end();itor++)
					{
						INiceNetServer *pInServer = itor->second;
						pInServer->stop(true);
					}
				}
            }

			NETLOG_NORMAL("begin close work thread...!"<<FUN_FILE_LINE);
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);           
			PNGB::m_pLog->ClearLogGrade(Log_PutOutScreen);
			INiceLog::DeleteLog( PNGB::m_pLog );
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);

			NETLOG_NORMAL("end call..."<<FUN_FILE_LINE);
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);		
			PNGB::m_pNetLogOpcTool->UnInit();
        }
        unsigned CNiceNet::Run( unsigned uCount,unsigned cur_time, unsigned cur_timeSs,unsigned &doMsgCt )
        {
			int iHappenTryPos = 1000;
			try
			{
				if( uCount<=0 )
					uCount = 1000;
				if(0==cur_time || 0==cur_timeSs)
				{
					printf("Cur_time cur_timeSs must isnot zero!!!!");
					return 0;
				}
				PNGB::m_server_curTm      = cur_time;
				PNGB::m_server_relativeTm = cur_timeSs;
				if( CLimit::is_runMaxDuration() )
				{
					//NETLOG_FATAL("[RunMaxDuration!] CLimit:"<<FUN_FILE_LINE );
					boost::this_thread::sleep( boost::posix_time::milliseconds(1000 ) );
					return 0;
				}
				iHappenTryPos = 1040;
				OnTimeCheckHeartTimeout();
				iHappenTryPos = 1045;
				OnTimeCheckStopAccept();
				iHappenTryPos = 1050;

				//Boost_Scoped_Lock NiceNet_lock(m_RunMsgmutex);
				OnTime_1000Millisecond( cur_timeSs );

				if( PNGB::m_pNetLogOpcTool)
					PNGB::m_pNetLogOpcTool->run();

				iHappenTryPos = 1060;
				doMsgCt = 0;
				if( PNGB::m_pNiceMsgMG ){
					doMsgCt = PNGB::m_pNiceMsgMG->Run(uCount,cur_time);
				}
			}
			catch (const std::exception&)
			{
				NETLOG_ERROR("[发生了异常.....] iHappenTryPos=" << iHappenTryPos << FUN_LINE);
			}
			if( doMsgCt>=uCount )
				return true;
			if( cur_time>PNGB::m_zhtc_slowertime )
				return true; //比较忙
			return false;
        }

		void CNiceNet::OnTime_1000Millisecond( unsigned cur_timeSs )
		{
			static  unsigned last_runtime = cur_timeSs;
			if( cur_timeSs-last_runtime>10 )
			{
				//NETLOG_ERROR( "Abbb...last_runtime="<<last_runtime<<";cur_timeSs="<<cur_timeSs<<FUN_LINE );
				last_runtime = cur_timeSs;

				//Boost_Scoped_Lock NiceNet_lock(m_mutex);
				PNGB::m_pNetInCounterMg->OnInCounterTime();
				if( PNGB::m_pNiceMsgMG )
					PNGB::m_pNiceMsgMG->UpdateCounter();
				this->SendMsg_CounterStateInfo();
			}
		}

        unsigned CNiceNet::AddClient(TNewClientParam &param)
        {
			if( CLimit::is_expired() )
			{
				NETLOG_ERROR("[..........!] CLimit:"<<FUN_FILE_LINE );
				return 0;
			}

            if( !m_IsInitOk )
            {
                return INVALID_CONN_INDEX;
            }
            //	create connection to a server
            unsigned connectid=0;
            if(EmPNetSType_Tcp==param.iNetType)
            {
                connectid=PNGB::m_pTCPClient->connect_to(param);
            }
			else if(EmPNetSType_WebSocket == param.iNetType)
			{
				connectid = PNGB::m_pWSClient->connect_to(param);
			}
			else if(EmPNetSType_Udp == param.iNetType)
            {
                connectid=PNGB::m_pUDPClient->connect_to(param);
            }
            NETLOG_NORMAL(__FUNCTION__<<"(): Connectid="<<connectid<<"; ip="<<param.strServerIp<<"; port="<<param.strServerPort);
            return connectid;
        }

        unsigned CNiceNet::AddServer(TNewServerParam &param)
        {
			if( CLimit::is_expired() )
			{
				NETLOG_ERROR("[..........!!] CLimit:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}

			if( !is_right_ipstr(param.strServerIp) )
			{
				NETLOG_ERROR("strServerIp format error:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}
			//NETLOG_SYSINFO("[创建 TcpServer];param:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);

            //Boost_Scoped_Lock NiceNet_lock(m_mutex);
            if( !m_IsInitOk )
            {
                return 0;
            }
            TCPServer *pserver = new TCPServer(param,boost::bind(&CNiceNet::NoticeDelServer,this,_1) );
			m_mapNetServer[pserver->GetID()] = pserver;
            if( false == pserver->StartSocket() ){
				pserver->stop( true );
				return 0;
			}
			//NETLOG_SYSINFO("[创建一个 TcpServer]: ListenIpPort=" << pserver->GetListenIpPort() << this->GetCurThreadIdStr());
			NETLOG_SYSINFO("[创建一个 TcpServer]: ListenIpPort="<<pserver->GetListenIpPort() << "; maxcount:" << param.uMaxConnectCount);
			NETLOG_SAVELOG();
			//20101109
			PNGB::m_pNetInCounterMg->AddOrDelServer( pserver->IsHttpServer(), pserver->GetID(),true);
            return pserver->GetID();
        }

        unsigned CNiceNet::AddUdpServer(TNewServerParam &param)
        {
            UDPServer *pserver = new UDPServer(param, boost::bind(&CNiceNet::NoticeDelServer, this, _1));
            m_mapNetServer[pserver->GetID()] = pserver;
            if(false == pserver->StartSocket())
            {
                pserver->stop(true);
                return 0;
            }
			NETLOG_SYSINFO("[创建一个UdpServer]: ListenIpPort="<<pserver->GetListenIpPort() << "; maxcount:" << param.uMaxConnectCount);
            NETLOG_SAVELOG();
            //20101109
            PNGB::m_pNetInCounterMg->AddOrDelServer(pserver->IsHttpServer(), pserver->GetID(), true);
            return pserver->GetID();
        }

        unsigned CNiceNet::AddHttpServer(TNewServerParam &param)
		{
			if( CLimit::is_expired() )
			{
				//NETLOG_ERROR("[..........!!] CLimit:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}

			if( !is_right_ipstr(param.strServerIp) )
			{
				NETLOG_ERROR("strServerIp format error:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}

			//Boost_Scoped_Lock NiceNet_lock(m_mutex);
			if( !m_IsInitOk )
			{
				return 0;
			}
			HttpServer *pserver = new HttpServer(param,boost::bind(&CNiceNet::NoticeDelServer,this,_1) );
			if( false == pserver->StartSocket())
			{
				//delete pserver;
				//pserver = 0;
				pserver->stop( true );
				return 0;
			}
			m_mapNetServer[pserver->GetID()] = pserver;
			NETLOG_SYSINFO("[创建一个 httpServer];param:"<<pserver->GetListenIpPort()<<"; maxcount:"<<param.uMaxConnectCount);
			NETLOG_SAVELOG();

			//20101109
			PNGB::m_pNetInCounterMg->AddOrDelServer(pserver->IsHttpServer(), pserver->GetID(),true);
			return pserver->GetID();
		}

		bool CNiceNet::DelClient(unsigned uClientID, string strWhy)
        {
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(uClientID, IDKind, uServer_instance_id))
			{
				NETLOG_ERROR("[逻辑错误 ] uClientID="<< uClientID<<", Why="<<strWhy<<FUN_LINE );
				return false;
			}

			if(IDKIND_CLIENTTCP == IDKind)
			{
				PNGB::m_pTCPClient->disconnect(uClientID, strWhy);
			}
			else if(IDKIND_CLIENTWS == IDKind)
			{
				PNGB::m_pWSClient->disconnect(uClientID, strWhy);
			}
			else if(IDKIND_CLIENTUDP == IDKind)
			{
				PNGB::m_pUDPClient->disconnect(uClientID, strWhy);
			}
            return true;
        }
        bool CNiceNet::DelServer( unsigned uServerID )
        {
            //Boost_Scoped_Lock NiceNet_lock(m_mutex);
			INiceNetServer *pserver = this->FindServer( uServerID );
            if( pserver)
            {
				NETLOG_SYSINFO("[开始删除一个Server]: ListenIpPort="<<pserver->GetListenIpPort()<<this->GetCurThreadIdStr()<<FUN_FILE_LINE );
                pserver->stop( true );
                return true;
            }
            return false;
        }
		void CNiceNet::CloseConByID(unsigned uConID,string strWhy/*="AppCallClose"*/)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConID,IDKind,uServer_instance_id) )
			{
				if( IDKIND_SERVER_SUBCLIENT==IDKind )
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
					if( itor!=m_mapNetServer.end() )
					{
						itor->second->CloseSub( uConID,strWhy );
					}
				}else
				{
					DelClient( uConID,strWhy );
                }
			}
		}

        void CNiceNet::NoticeDelServer(INiceNetServer * pserver)
        {
			NETLOG_SYSINFO("[启动删除一个Server]: ListenIpPort="<<pserver->GetListenIpPort()<<this->GetCurThreadIdStr()<<FUN_FILE_LINE );
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(&IAppServer::ServerStatus,pserver->GetIAppServer(),pserver->GetID(),IAppServer::sc_server_status_close) );
			PNGB::m_pNiceMsgMG->PutMsgFun( boost::bind(&CNiceNet::PerformDelServer,this,pserver->GetID()) );
            
            //把这个函数也放到主逻辑的执行队列里面就可以了
			//PNGB::m_pScheduler->post(boost::bind(&CNiceNet::PerformDelServer,this,pserver) );
			NETLOG_SAVELOG();
        }
        void CNiceNet::PerformDelServer( unsigned uServerID )
        {
			INiceNetServer *pserver = this->FindServer( uServerID );
			if( !pserver )
				return;

			NETLOG_SYSINFO("[删除一个Server]: ListenIpPort="<<pserver->GetListenIpPort()<<this->GetCurThreadIdStr()<<FUN_FILE_LINE );
			NETLOG_SAVELOG();
			if( 0==pserver->GetDelStartTm() )
				return;
			if( PNGB::m_server_curTm-pserver->GetDelStartTm()<5 )
				return;
            //Boost_Scoped_Lock NiceNet_lock(m_mutex);
			PNGB::m_pNetInCounterMg->AddOrDelServer( pserver->IsHttpServer(),pserver->GetID(),false);
            m_mapNetServer.erase(pserver->GetID());
            delete pserver;
			NETLOG_SAVELOG();
        }

        void CNiceNet::LogoutCounter( CNiceNetCounter *pReg_info )
        {
            //Boost_Scoped_Lock NiceNet_lock(m_mutex);
            LIST_UInt BookItUserList;
            PNGB::m_pCountersMg->UnRegisterCounter( *pReg_info,BookItUserList );
            INPROTOCOL::TServerRepBook cmd;
            cmd.counter_id = pReg_info->GetID();
            cmd.is_success = 0;
            for(LIST_UInt::iterator itor=BookItUserList.begin(); itor!=BookItUserList.end(); ++itor)
                this->SendMsg( *itor,&cmd,sizeof(cmd) );
        }
        void CNiceNet::LogoutAllCounters()
        {
            LIST_UInt BookItUserList;
            LIST_UInt BookItCoureridList;
            PNGB::m_pCountersMg->UnRegisterAllCounter(BookItUserList,BookItCoureridList );
            if( BookItUserList.size()!=BookItCoureridList.size() )
			{
				NETLOG_ERROR("BookItUserList.size="<<BookItUserList.size()<<",BookItCoureridList.size="<<BookItCoureridList.size()<<FUN_FILE_LINE);
			}
            INPROTOCOL::TServerRepBook cmd;
            cmd.is_success = 0;
            LIST_UInt::iterator itorCt= BookItCoureridList.begin();
            LIST_UInt::iterator itor  = BookItUserList.begin();
            for( ;(itor!=BookItUserList.end()) && (itorCt!=BookItCoureridList.end()); ++itor,++itorCt)
            {
                cmd.counter_id = *itorCt;
                this->SendMsg( *itor,&cmd,sizeof(cmd) );
            }
        }
        bool CNiceNet::SendMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool ischeck/*=true*/)
        {
			if( !pdata )
				return false;
			 
			static unsigned Sc_uMaxPack = 1024*1024*10; //10M
			if( data_len>Sc_uMaxPack )
			{
				NETLOG_ERROR("你发送的数据包太大,长度为:"<<data_len);
				return false;
			}

			//检查数据头是否正确
			tcp_pak_header *pCheck = (tcp_pak_header*)pdata;
			if( ischeck )
			{
				unsigned uTMsgLenAa = (unsigned)pCheck->GetLen();
				if(  (uTMsgLenAa!=data_len) || (uTMsgLenAa<sc_pak_header_len)  )
				{
					unsigned uPort=0;
					string romeip = this->GetRemoteIp(uPort,uConnectid );
					
					NETLOG_ERROR("[NetSendMsgFail 消息包头逻辑错误]: MsgID="<<pCheck->GetMsgID()<<"; uTMsgLenAa="<<pCheck->Len<<"; UserID="<<pCheck->UserID<<", [ romeip="<<romeip<<":"<<uPort<<" ]"<<FUN_FILE_LINE );
					return false;

				}

				if(  uTMsgLenAa>Sc_uMaxPack ){
					NETLOG_ERROR("[NetSendMsgFail 消息包太长了]["<<pCheck->GetMsgID()<<",MsgLen="<<uTMsgLenAa<<FUN_FILE_LINE );
					return false;

				}
			}
            if(pCheck->GetMsgID() ==  1997699500){
                NETLOG_ERROR("[NetSendMsgFailAaaa]"<<pCheck->GetMsgID()<<FUN_LINE);
            }

			//pCheck->BackID     = 0;
			//pCheck->CheckCode= sc_THttpCheckCode;
			if( PNGB::m_IsCheckPackHead )
				pCheck->SetCheckCode();
			if( PNGB::m_isbegin_closesys )
				return false;

			int iErrID = 6901014;
            EmIDKind IDKind;
            unsigned uServer_instance_id=0;
            if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid,IDKind,uServer_instance_id) )
            {
                if( IDKIND_SERVER_SUBCLIENT==IDKind )
                {
                    //Boost_Scoped_Lock NiceNet_lock(m_mutex);
                    MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
                    if( itor==m_mapNetServer.end() )
                    {
                        NETLOG_ERROR(FUN_LINE<<"();server no find:invalid index:"<<uConnectid);
                        return false;
                    }
                    iErrID = itor->second->SendMsg(uConnectid,pdata,data_len);
                }else if( IDKIND_CLIENTTCP == IDKind )
                {
                    iErrID = PNGB::m_pTCPClient->send(uConnectid,pdata,data_len);
                }else if(IDKIND_CLIENTUDP == IDKind)
                {
                    iErrID = PNGB::m_pUDPClient->send(uConnectid, pdata, data_len);
                }
            }
			if( iErrID>0 )
			{
				NETLOG_NORMAL("SendMsgFail: errid="<<iErrID<<";uConnectid="<<uConnectid<<FUN_LINE );
			}
            return (0==iErrID);
        }

		bool CNiceNet::SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText)
		{
			if( PNGB::m_isbegin_closesys )
				return false;

			int iErrID = 0;
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid,IDKind,uServer_instance_id) )
			{
				if( IDKIND_SERVER_SUBCLIENT==IDKind )
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
					if( itor==m_mapNetServer.end() )
					{
						NETLOG_ERROR(FUN_LINE<<"();server no find:invalid index:"<<uConnectid);
						return false;
					}
					iErrID = itor->second->SendWebSocketMsg(uConnectid,pdata,data_len,IsText);
				}else if( IDKIND_CLIENTWS == IDKind )
				{
					iErrID = PNGB::m_pWSClient->SendWebSocketMsg( uConnectid, pdata, data_len,IsText );
				}
			}
			if( iErrID>0 )
			{
				NETLOG_ERROR("SendMsgFail: errid="<<iErrID<<";uConnectid="<<uConnectid<<FUN_LINE );
			}
			return (0==iErrID);
		}
		bool CNiceNet::IsHttpCon(unsigned uConID)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if (PNGB::m_pCSIDMG->GetIndexIDByFactID(uConID, IDKind, uServer_instance_id))
			{
				if (IDKIND_SERVER_SUBCLIENT == IDKind)
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
					if (itor == m_mapNetServer.end()) {
						return false;
					}
					return itor->second->IsHttpServer();
				}
			}
			return false;
		}

		bool CNiceNet::IsWssCon(unsigned uConID)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(PNGB::m_pCSIDMG->GetIndexIDByFactID(uConID,IDKind,uServer_instance_id))
			{
				if(IDKIND_SERVER_SUBCLIENT == IDKind)
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
					if(itor == m_mapNetServer.end()) {
						return false;
					}
					return itor->second->IsWssCon();
				}
			}
			return false;
		}

		bool CNiceNet::IsTcpCon(unsigned uConID)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if (PNGB::m_pCSIDMG->GetIndexIDByFactID(uConID, IDKind, uServer_instance_id))
			{
				if (IDKIND_SERVER_SUBCLIENT == IDKind)
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
					if (itor == m_mapNetServer.end()) {
						return false;
					}
					return itor->second->IsTcpCon();
				}
			}
			return false;
		}

		IHttpReq * CNiceNet::GetHttpReq(unsigned uConID)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConID,IDKind,uServer_instance_id) )
			{
				if( IDKIND_SERVER_SUBCLIENT==IDKind )
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
					if( itor==m_mapNetServer.end() ){
						return 0;
					}
					return itor->second->GetHttpReq(uConID);
				}
			}
			return 0;
		}

		unsigned CNiceNet::AddWebSocketServer(TNewServerParam &param)
		{
			if( CLimit::is_expired() )
			{
				//NETLOG_ERROR("[..........!!] CLimit:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}

			if( !is_right_ipstr(param.strServerIp) )
			{
				NETLOG_ERROR("strServerIp format error:"<<"strServerIp:"<<param.strServerIp<<";port:"<<param.uServerPort<<"; maxcount:"<<param.uMaxConnectCount);
				return 0;
			}

			//Boost_Scoped_Lock NiceNet_lock(m_mutex);
			if( !m_IsInitOk )
			{
				return 0;
			}
			WebSocketServer *pserver = new WebSocketServer(param,boost::bind(&CNiceNet::NoticeDelServer,this,_1) );
			if( false == pserver->StartSocket())
			{
				pserver->stop( true );
				return 0;
			}
			m_mapNetServer[pserver->GetID()] = pserver;
			NETLOG_SYSINFO("[创建一个 WebSocketServer];param:"<<pserver->GetListenIpPort()<<"; maxcount:"<<param.uMaxConnectCount);
			NETLOG_SAVELOG();

			//20101109
			PNGB::m_pNetInCounterMg->AddOrDelServer(pserver->IsHttpServer(), pserver->GetID(),true);
			return pserver->GetID();
		}

		string CNiceNet::GetRemoteIp(unsigned &uPort,unsigned uConnectid)
		{
			uPort = 0;
			//return "127.0.0.1";
			uPort = 0;
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid, IDKind, uServer_instance_id))
			{
				return "";
			}
			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				//Boost_Scoped_Lock NiceNet_lock(m_mutex);
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
				{
					return "";
				}
				return itor->second->GetRemoteIp(uPort, uConnectid);
			}

			IFaceXClientMg *pXCFace = FindXClientMg(uConnectid, IDKind);
			if(pXCFace)
			{
				return pXCFace->GetRemoteIp(uPort, uConnectid);
			}
			return "";
		}

		string CNiceNet::GetRemoteIp(unsigned uConnectid)
		{
			unsigned port;
			return GetRemoteIp( port,uConnectid );
		}
		string CNiceNet::GetLocalIp(unsigned &uPort,unsigned uConnectid)
		{
			uPort = 0;
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid, IDKind, uServer_instance_id))
			{
				return "";
			}
			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				//Boost_Scoped_Lock NiceNet_lock(m_mutex);
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
				{
					return "";
				}
				return itor->second->GetLocalIp(uPort, uConnectid);
			}

			IFaceXClientMg *pXCFace = FindXClientMg(uConnectid, IDKind);
			if(pXCFace)
			{
				return pXCFace->GetLocalIp(uPort, uConnectid);
			}
			return "";
		}

		string CNiceNet::GetLocalIp(unsigned uConnectid)
		{
			unsigned port;
			return GetLocalIp( port,uConnectid );
		}
		string CNiceNet::GetServerIpPortBySubConID(unsigned subconid, unsigned &port )
		{
			port = 0;
			//return "127.0.0.1";

			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(subconid,IDKind,uServer_instance_id) )
			{
				if( IDKIND_SERVER_SUBCLIENT==IDKind )
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
					if( itor==m_mapNetServer.end() )
					{
						NETLOG_ERROR("[逻辑错误] aa subconid="<<subconid<<FUN_FILE_LINE );
						return "";
					}
					TCPServer *pserver = (TCPServer*)itor->second;
					port = pserver->m_param.uServerPort;
					return pserver->m_param.strServerIp;
				}else
				{
					NETLOG_ERROR("[逻辑错误] bb subconid="<<subconid<<FUN_FILE_LINE );
					return "";
				}
			}
			NETLOG_ERROR("[逻辑错误] cc subconid="<<subconid<<FUN_FILE_LINE );
			return "";
		}

		void CNiceNet::SetCurAppMainCallPos(string strCallPos,unsigned iMaxTmLen)
		{
			m_MAppCallPos.uBeginCallTm = CInPubFun::GetServerRelative();
			m_MAppCallPos.uRunTmMaxLen = iMaxTmLen;
			CInPubFun::strcpy( m_MAppCallPos.strPos,sizeof(m_MAppCallPos.strPos),strCallPos.c_str(),93203 );
		}

		string CNiceNet::CheckAppMainThreadIsLock()
		{//这个函数在子线程中运行

			//<001> 这个函数依靠不停的重置uBeginCallTm来实现，这个数值如果长时间没有被重置，就说明设置这个函数的调用发生了问题。

			PNGB::m_server_curTm      = CInPubFun::GetNowTime();
			PNGB::m_server_relativeTm = CInPubFun::GetServerRelative();

			//控制不要频繁的进行逻辑处理,上次检查的时间
			if( PNGB::m_server_relativeTm<m_MAppCallPos.uLastCheckTm)
				return "";
			m_MAppCallPos.uLastCheckTm = PNGB::m_server_relativeTm+m_MAppCallPos.uRunTmMaxLen;
			

			//多个线程读，只有主逻辑写
			if( 0==m_MAppCallPos.uBeginCallTm ) //好像不可能存在,容错
				return "";
			if( PNGB::m_server_relativeTm<m_MAppCallPos.uBeginCallTm )//好像不可能存在,容错
				return "";

			//检查这个函数运行的时间是否太长了
			unsigned iRunTmLen = PNGB::m_server_relativeTm-m_MAppCallPos.uBeginCallTm; //这个函数运行的时间
			if( iRunTmLen<m_MAppCallPos.uRunTmMaxLen )
				return "";
			string strPos = m_MAppCallPos.strPos;
			if( false == strPos.empty() ){
				NETLOG_ERROR("MainAppLogic deathLock....: "<<strPos<<"; BeginCallTm="<<m_MAppCallPos.uBeginCallTm<<"; RunTmLen="<<iRunTmLen<<"; RunTmMaxLen="<<m_MAppCallPos.uRunTmMaxLen);
			}
			return strPos;
		}

		unsigned CNiceNet::GetMaxCounterCount(unsigned uConnectid)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid, IDKind, uServer_instance_id))
			{
				return 0;
			}
			if(IDKIND_SERVER == IDKind)
			{
				//Boost_Scoped_Lock NiceNet_lock(m_mutex);
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
					return 0;
				return itor->second->GetMaxCounterCount();
			}
			return 0;
		}

		unsigned CNiceNet::GetSeverCurConnectCount(unsigned uConnectid)
		{
			if( uConnectid>0 )
			{
				EmIDKind IDKind;
				unsigned uServer_instance_id;
				if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid,IDKind,uServer_instance_id) )
				{
					if( IDKIND_SERVER==IDKind )
					{
						//Boost_Scoped_Lock NiceNet_lock(m_mutex);
						MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
                        if(uConnectid == itor->first)
                        {
                            INiceNetServer *pNNServer = itor->second;
                            return pNNServer->GetSeverCurConnectCount();
                        }
                        return 0;
					}
				}
			}else
			{
				unsigned uCountTt = PNGB::m_pTCPClient->GetClientTotalCount();
				for(MAP_NETSERVER::iterator itor = m_mapNetServer.begin();itor!=m_mapNetServer.end();itor++)
				{
					INiceNetServer *pInServer = itor->second;
					uCountTt += pInServer->GetSeverCurConnectCount();
				}
				return uCountTt;
			}
			return 0;
		}

		void Gfun_Log_RepCounterState( INPROTOCOL::TServerNoticeState *pMsg )
		{
			ostringstream strLog;
			for ( int t=0;t<pMsg->count; t++ )
			{
				strLog<<"( "<<pMsg->items[t].counter_id<<","<<pMsg->items[t].value<<" );";
			}
			NETLOG_ERROR( "Gfun_Log_RepCounterState:"<<strLog.str() );
		}
        void CNiceNet::SendMsg_CounterStateInfo()
        {
            MAP_UserBookCounterSate mapListMsg;
            PNGB::m_pCountersMg->GetCounterStateList(mapListMsg);
            for(MAP_UserBookCounterSate::iterator itor=mapListMsg.begin(); itor!=mapListMsg.end(); ++itor)
            {
                INPROTOCOL::LIST_RepCounterState & ListRepSt        = itor->second;
                INPROTOCOL::LIST_RepCounterState::iterator itorList = ListRepSt.begin();
                for(;itorList!=ListRepSt.end();++itorList)
                {
                    INPROTOCOL::TServerNoticeState *pMsg = &(*itorList);
                    unsigned iSendLen = sizeof(INPROTOCOL::TServerNoticeState);
                    iSendLen -= sizeof(INPROTOCOL::TServerNoticeState::COUNTER_ITEM_STATE)*(INPROTOCOL::COUNTERLIST_MAXCOUNT-pMsg->count);
                    //pMsg->Len = iSendLen;
					pMsg->SetLen( iSendLen );
					this->SendMsg(itor->first,pMsg,iSendLen);

					//Gfun_Log_RepCounterState( pMsg );
                }
            }
        }
		bool CNiceNet::GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid, IDKind, uServer_instance_id))
			{
				return false;
			}
			
			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				//Boost_Scoped_Lock NiceNet_lock(m_mutex);
				MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
				if( itor==m_mapNetServer.end() )
				{
					return false;
				}
				return itor->second->Server_GetSessionBufferInfo( uConnectid,info );
			}
		
			IFaceXClientMg *pXCFace = FindXClientMg(uConnectid, IDKind);
			if(pXCFace)
			{
				return pXCFace->ClientS_GetSessionBufferInfo(uConnectid, info);
			}

			return false;
		}
		/*
		bool CNiceNet::ClearSeeionBufferInfo(unsigned uConnectid)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(uConnectid,IDKind,uServer_instance_id) )
			{
				if( IDKIND_SERVER_SUBCLIENT==IDKind )
				{
					//Boost_Scoped_Lock NiceNet_lock(m_mutex);
					MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
					if( itor==m_mapNetServer.end() )
					{
						return false;
					}
					return itor->second->Server_ClearSeeionBufferInfo( uConnectid );
				}else if( IDKIND_CLIENT == IDKind )
				{
					return PNGB::m_pTCPClient->ClientS_ClearSeeionBufferInfo(uConnectid );
				}
			}
			return false;
		}
		*/

		bool CNiceNet::AddCounterForCon(unsigned ucondid,string strdes)
		{
			//return false;
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( PNGB::m_pCSIDMG->GetIndexIDByFactID(ucondid,IDKind,uServer_instance_id) )
			{
				PNGB::m_pNetInCounterMg->AddTcpSession( ucondid );
				return true;
			}
			return false;
		}
		bool CNiceNet::DelCounterForCon( unsigned ucondid )
		{
			PNGB::m_pNetInCounterMg->DelTcpSession( ucondid );
			return true;
		}
		int CNiceNet::AddAllocatee( unsigned ubuffersize,unsigned maxcount )
		{
			return PNGB::m_pAllocMg->add_alloc( ubuffersize,maxcount);
		}

		//新添加的20101109
		INiceNetServer * CNiceNet::FindServer( unsigned serverinstanceid )
		{
			MAP_NETSERVER::iterator itor = m_mapNetServer.find( serverinstanceid );
			if( itor==m_mapNetServer.end() )
				return 0;
			return itor->second;
		}

		peony::net::IFaceXClientMg * CNiceNet::FindXClientMg(unsigned iCcConID, EmIDKind IDKind)
		{
			if( IDKIND_NODEF == IDKind )
			{
				unsigned uServer_instance_id;
				if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(iCcConID, IDKind, uServer_instance_id))
					return 0;
			}
			
			if(IDKIND_CLIENTTCP == IDKind)
			{
				return PNGB::m_pTCPClient;
			}
			else if(IDKIND_CLIENTUDP == IDKind)
			{
				return PNGB::m_pUDPClient;
			}
			else if(IDKIND_CLIENTWS == IDKind)
			{
				return PNGB::m_pWSClient;
			}
			return 0;
		}

		void CNiceNet::SetXAttrib(unsigned conid, unsigned xadd, unsigned xdel)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if( false == PNGB::m_pCSIDMG->GetIndexIDByFactID(conid,IDKind,uServer_instance_id) )
			{
				return;
			}

			if( IDKIND_SERVER_SUBCLIENT == IDKind )
			{
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
					return;
				itor->second->SetXAttrib(conid, xadd, xdel);
				return;
			}

			IFaceXClientMg *pXCFace = FindXClientMg(conid, IDKind);
			if(pXCFace)
			{
				pXCFace->SetXAttrib(conid, xadd, xdel);
			}
		}
		bool CNiceNet::IsXAttrib(unsigned conid,unsigned xmark)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(conid, IDKind, uServer_instance_id))
			{
				return false;
			}

			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
					return false;
				return itor->second->IsXAttrib(conid, xmark);
			}
			IFaceXClientMg *pXCFace = FindXClientMg(conid, IDKind);
			if(pXCFace)
			{
				return pXCFace->IsXAttrib(conid, xmark);
			}
			return false;
		}

		bool CNiceNet::is_right_ipstr( string strip )
		{
			vector< string > vctResult;
			boost::split( vctResult, strip, boost::is_any_of(".") );
			if( vctResult.size()!=4 )
				return false;
			
			for( unsigned t=0; t<vctResult.size(); ++t )
			{
				try
				{
					if( !boost::algorithm::all( vctResult[t], boost::is_digit() ) )
						return false;
					if( boost::lexical_cast<int>(vctResult[t])>255 )
						return false;
				}
				catch (...)
				{
					return false;
				}
			}

			return true;
		}

		void CNiceNet::SetConData(unsigned conid,uint64_t udata)
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(conid, IDKind, uServer_instance_id))
			{
				return;
			}
			if( IDKIND_SERVER_SUBCLIENT==IDKind )
			{
				MAP_NETSERVER::iterator itor = m_mapNetServer.find( uServer_instance_id );
				if( itor==m_mapNetServer.end() )
					return;
				itor->second->SetConData(conid,udata);
				return;
			}

			IFaceXClientMg *pXCFace = FindXClientMg(conid, IDKind);
			if(pXCFace)
			{
				pXCFace->SetConData(conid, udata );
			}
		}

		uint64_t CNiceNet::GetConData( unsigned conid )
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(conid, IDKind, uServer_instance_id))
			{
				return false;
			}
			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
					return 0;
				return itor->second->GetConData(conid);
			}

			IFaceXClientMg *pXCFace = FindXClientMg(conid, IDKind);
			if(pXCFace)
			{
				return pXCFace->GetConData(conid);
			}
			return 0;
		}

		void CNiceNet::OnTimeCheckHeartTimeout()
		{
			if( PNGB::m_server_curTm-m_LastCheckHeartTm<2 )
				return;
			m_LastCheckHeartTm = PNGB::m_server_curTm;

			LIST_UInt listConID;
			MAP_NETSERVER::iterator itor = m_mapNetServer.begin();
			for( ;itor!=m_mapNetServer.end(); itor++ ){
				itor->second->OnTimeCheckHeartTimeout( listConID );
			}
			if( listConID.size()>0 )
			{
				//NETLOG_SYSINFO( "listConIDSize="<<listConID.size()<<FUN_FILE_LINE );
			}
			
			if(PNGB::m_pTCPClient)
			{
				PNGB::m_pTCPClient->OnTimeCheckHeartTimeout(listConID);
				if(listConID.size()>0){
					//NETLOG_SYSINFO("TcpListConIDSize="<<listConID.size()<<FUN_FILE_LINE);
				}
			}
			if(PNGB::m_pUDPClient)
			{
				PNGB::m_pUDPClient->OnTimeCheckHeartTimeout(listConID);
				if( listConID.size() > 0 ){
					//NETLOG_SYSINFO("CheckTimeOut.UdpListConIDSize=" << listConID.size() << FUN_FILE_LINE);
				}
			}
			if(PNGB::m_pWSClient)
			{
				PNGB::m_pWSClient->OnTimeCheckHeartTimeout(listConID);
				if(listConID.size() > 0) {
					//NETLOG_SYSINFO("CheckTimeOut.WsListConIDSize=" << listConID.size() << FUN_FILE_LINE);
				}
			}

			if( listConID.empty() )
				return;
			LIST_UInt::iterator itb = listConID.begin();
			for (; itb!=listConID.end(); itb++ )
			{
				unsigned uTempConID = *itb;
				this->SetXAttrib( uTempConID,socketmk_IsHeartTimeout,0 );
				this->CloseConByID( uTempConID,"PNetCheckHeartTimeOut" );
			}
		}

		void CNiceNet::OnTimeCheckStopAccept()
		{
			if( PNGB::m_server_curTm-m_LastCheckAcceptTm<5 )
				return;
			m_LastCheckAcceptTm = PNGB::m_server_curTm;

			MAP_NETSERVER::iterator itor = m_mapNetServer.begin();
			for( ;itor!=m_mapNetServer.end(); itor++ )
			{
				INiceNetServer *pXSs = itor->second;
				pXSs->CheckIsStopAccept();
			}

		}

		bool CNiceNet::IsValidConid( unsigned conid )
		{
			EmIDKind IDKind;
			unsigned uServer_instance_id;
			if(false == PNGB::m_pCSIDMG->GetIndexIDByFactID(conid, IDKind, uServer_instance_id))
			{
				return false;
			}
			if(IDKIND_SERVER_SUBCLIENT == IDKind)
			{
				MAP_NETSERVER::iterator itor = m_mapNetServer.find(uServer_instance_id);
				if(itor == m_mapNetServer.end())
					return false;
				return true;
			}
			IFaceXClientMg *pXCFace = FindXClientMg(conid, IDKind);
			if(pXCFace)
			{
				return pXCFace->IsValidConid(conid);
			}
			return false;
		}

		void CNiceNet::SetLogPutGrade( unsigned iSet )
		{
			return PNGB::m_pLog->SetLogPutGrade( iSet );
		}
		unsigned CNiceNet::GetLogPutGrade()
		{
			return PNGB::m_pLog->GetLogGrade();
		}

		void CNiceNet::RealTimeWriteLog()
		{
			PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
		}
		void CNiceNet::OnTime_debug_serverthread()
		{//为了寻找一个线程
			//unsigned theadid = GetCurrentThreadId(); //CInPubFun::GetThreadID( 2130213 );
			//NETLOG_SYSINFO(theadid<<FUN_FILE_LINE);
			//unsigned aa = theadid;
		}
	}
}