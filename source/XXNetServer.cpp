#include "../header/Scheduler.hpp"
#include "../header/XXNetServer.hpp"

#include "../header/NiceIDMg.h"
#include "../header/CGlobal.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/CountersMg.hpp"
#include "../header/NetCountersInstanceMg.h"
#include "../header/CAllocMg.h"


namespace peony
{
	namespace net 
	{
		TCPServer::TCPServer( TNewServerParam &param,server_deallocator_type DelSelfFun) : TcpServerBase(param,DelSelfFun)
		{
		}

		TCPServer::~TCPServer()
		{
			NETLOG_ERROR("[析构完成.被删除结束 TCPServer], uServerID="<<GetID()<<"; MeListenIpPort="<<GetListenIpPort()<<FUN_FILE_LINE);
			PNGB::m_pCSIDMG->FreeID(m_instance_id);
		}


		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		HttpServer::HttpServer(TNewServerParam &param,server_deallocator_type DelSelfFun) : TcpServerBase(param,DelSelfFun)
		{

		}
		HttpServer::~HttpServer()
		{
			NETLOG_ERROR("[析构完成.被删除结束 HttpServer], uServerID="<<GetID()<<"; MeListenIpPort="<<GetListenIpPort()<<FUN_FILE_LINE);
			PNGB::m_pCSIDMG->FreeID(m_instance_id);
		}

		peony::net::IHttpReq * HttpServer::GetHttpReq(unsigned uConID)
		{
			HttpServerSession *pSession = GetXSessionByConID(uConID);
			if (!pSession)
				return 0;
			return pSession->GetHttpReq();
		}

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		WebSocketServer::WebSocketServer(TNewServerParam &param,server_deallocator_type DelSelfFun)	: TcpServerBase(param,DelSelfFun)
		{

		}
		WebSocketServer::~WebSocketServer()
		{
			NETLOG_ERROR("[析构完成.被删除结束 WebSocketServer], uServerID="<<GetID()<<"; MeListenIpPort="<<GetListenIpPort()<<FUN_FILE_LINE);
			PNGB::m_pCSIDMG->FreeID(m_instance_id);
		}

		int WebSocketServer::SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText)
		{
			WebSocketSession *pWss = this->GetXSessionByConID( uConnectid );
			if( 0==pWss )
				return 8903283;
			return pWss->SendWebSocketMsg( (const char*)pdata,data_len,IsText);
		}

        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        UDPServer::UDPServer(TNewServerParam &param, server_deallocator_type DelSelfFun) : UdpServerBase(param, DelSelfFun)
        {

        }

        UDPServer::~UDPServer()
        {

        }

    }// end of namespace net
}// end of namespace peony
