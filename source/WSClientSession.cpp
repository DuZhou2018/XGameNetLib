#include "../header/WSClientSession.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CGlobal.hpp"
#include "../include/NiceNetBase64.h"

#ifdef WIN32
#pragma warning(disable:4503)
#endif

namespace peony {
	namespace net {

		WSClientSession::WSClientSession(unsigned conn_index, 
			TNewClientParam &param,TXZTBaseInitPm &initPM, Scheduler *pShedule)
			:CWSSocketBase(pShedule->get_ioservice(), initPM), m_Param(param),m_Resolver( pShedule->get_ioservice() )
		{
			m_pSch = pShedule;
			m_conn_index     = conn_index;
			m_InFunConnect   = boost::bind(&WSClientSession::ImpPackIAppClientOnConnect,this,_1,_2,_3,_4);
			m_runmsg_handler = boost::bind(&IAppClient::OnRecive,param.pApp,_1,_2,_3);
			m_SockType       = INiceNetSocket::ESockType_Client;
			strcpy(m_SocketTypeName, "TcpClient");
			this->SetXMark(socketmk_client, 0);
		}

		WSClientSession::~WSClientSession(void)
		{
            //CAutoLogDeadlock  AutoLgDL( "Connectid=",this->GetConnectID(),__FUNCTION__ );
			switch(m_EmConnectionState)
			{
			case CS_RESOLVING:
				post_connection_handler_zhtcc(GetConnectIndex(),CI_ADDR_RESOVLE_FAILED,"Unknown error.");
				break;
			case CS_CONNECTING:
				post_connection_handler_zhtcc(GetConnectIndex(),CI_CONNECTION_FAILED,"Unknown error.");
				break;
			default:
				break;
			}			
		}

		void WSClientSession::post_connection_handler_zhtcc(unsigned conn_index, EmConnectInfo info, const std::string &msg)
		{
			mainrun_call_onconnected(0, conn_index, info, msg, shared_from_this());
		}
		bool WSClientSession::CheckHeartTiimeout()
		{
			//return this->CheckHeartTimeout(m_Param.uHeartTmSecond);
			return false;
		}
		void WSClientSession::ImpPackIAppClientOnConnect(unsigned uMyServerID, unsigned uConnectID, int status, string info)
		{
			m_Param.pApp->OnConnect(uConnectID, status, info);
		}

		void WSClientSession::handle_resolve_zhtcc(const ZBoostErrCode& err, tcp::resolver::iterator endpoint_iterator)
		{
			AsioStrand &tStrand = m_pSch->get_strand();
			if(!err)
			{
				post_connection_handler_zhtcc(GetConnectIndex(), CI_ADDR_RESOVLE_SUCCESSED, "address resolved successfully");

				// Attempt a connection to the first endpoint in the list. Each endpoint
				// will be tried until we successfully establish a connection.
				m_EmConnectionState = CS_CONNECTING;
				tcp::endpoint endpoint = *endpoint_iterator;

				GetTcpSocket()->async_connect(endpoint,
					tStrand.wrap(boost::bind(&INiceNetSocket::handle_connect_zhtcc, shared_from_this(),
						boost::asio::placeholders::error, ++endpoint_iterator)));
			}
			else
			{
				m_EmConnectionState = CS_NONE;
				post_connection_handler_zhtcc(GetConnectIndex(), CI_ADDR_RESOVLE_FAILED, "ZHT.resovle fail!");
				ostringstream strLog;
				strLog<<"Connecid:" << GetConnectIndex() << ";CI_ADDR_RESOVLE_FAILED; Err:" << err.message();
				string strAaa = strLog.str();
				NETLOG_ERROR( strAaa<<FUN_LINE );
			}

		}

		void WSClientSession::handle_connect_zhtcc(const ZBoostErrCode& err, tcp::resolver::iterator endpoint_iterator)
		{
			if( !err )
			{//连接成功了
				// The connection was successful.
				m_EmConnectionState = CS_CONNECTED;
				post_connection_handler_zhtcc(GetConnectIndex(), CI_Connect_Success, "connection established");
				OpenState();

				//self log
				SubFunBuildSelfLog();
			}
			else if(endpoint_iterator != tcp::resolver::iterator())
			{
				NETLOG_ERROR(__FUNCTION__ << " connecid:" << GetConnectIndex() << ";the endpoint fail;");
				// The connection failed. Try the next endpoint in the list.
				GetTcpSocket()->close();
				tcp::endpoint endpoint = *endpoint_iterator;
				AsioStrand &tStrand = m_pSch->get_strand();

				GetTcpSocket()->async_connect(endpoint,
					tStrand.wrap(boost::bind(&INiceNetSocket::handle_connect_zhtcc, shared_from_this(),
						boost::asio::placeholders::error, ++endpoint_iterator)));
			}
			else
			{
				NETLOG_ERROR("tcpclientsession connect fail! connecid:[" << GetConnectIndex() << "," << m_Param.strServerIp << "," << m_Param.strServerPort << "]; errorinfo:" << err.message() << FUN_LINE);
				m_EmConnectionState = CS_NONE;
				post_connection_handler_zhtcc(GetConnectIndex(), CI_CONNECTION_FAILED, "zht.connect fail!");
			}
		}
		bool WSClientSession::XxTcpBeginRecWork()
		{
			//发送捂手协议
			SendHandshakeMsgReq();

			//Client第一次要读取握手协议的返回
			ImpRecMaxLenMsg(EHReadType_LogicConnect, m_ConnectBuf, sizeof(m_ConnectBuf) - 10, 2);
			return true;
		}

		int WSClientSession::HandRead_LogicConnect(size_t bytes_transferred)
		{
			//NETLOG_SYSINFO("[收到捂手协议返回]"<< LogSelf() << m_ConnectBuf << FUN_LINE);
			this->SetXMark(XMWSocket_IsConOk);
			return 0;
		}

		void WSClientSession::SendHandshakeMsgReq()
		{
			//string strKey = "0123456789012345"; //16
			string strKey = boost::str(boost::format("%uAbcdefghigklmnopkqqxx") % CInPubFun::GetNowTime());
			strKey = strKey.substr(0, 16);
			char strDesKey[64] = { 0 };
			Base64::Base64_Encode(strDesKey, strKey.c_str(), (unsigned)strKey.size());

			ostringstream strHead;
			strHead << "GET / HTTP / 1.1 \r\n";
			strHead << "Connection: keep - alive, Upgrade\r\n";
			strHead << "sec-websocket-version: 13\r\n";
			strHead << "sec-websocket-key: " << strDesKey << "\r\n";
			strHead << "Upgrade: websocket\r\n\r\n";
			CInPubFun::strcpy(m_ConnectBuf, sizeof(m_ConnectBuf), strHead.str().c_str(), 5210143);

			char    *pHttpSend = m_ConnectBuf;
			unsigned uData_len = (unsigned)strlen(m_ConnectBuf);
			int iErrID = this->XBase_WriteData(pHttpSend, uData_len, uData_len, 0);
			if( iErrID>0 )
			{
				NETLOG_ERROR("[发送握手协议失败]" << LogSelf()<<", ErrID="<<iErrID << FUN_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("SendMsgException1!");
				return;
			}
		}

		void WSClientSession::StartSocket()
		{
			// Start an asynchronous resolve to translate the server and service names
			// into a list of endpoints.
			tcp::resolver::query query(m_Param.strServerIp, m_Param.strServerPort);
			AsioStrand &tStrand = m_pSch->get_strand();

			//NETLOG_SYSINFO("[创建线程.一个奇怪的线程] threadid=0" );
			//下面的语句会创建一个线程，为什么，我还没有明白. 2017-03-25  zhanghongtao
			m_EmConnectionState = CS_RESOLVING;

			auto tFun = boost::bind(&INiceNetSocket::handle_resolve_zhtcc, shared_from_this(),
				                        boost::asio::placeholders::error, boost::asio::placeholders::iterator);
			m_Resolver.async_resolve( query,tStrand.wrap(tFun));
		}

		bool WSClientSession::OnRecive(unsigned uConnID, const void *pData, size_t data_len)
		{
			return m_Param.pApp->OnRecive(uConnID, pData, (unsigned)data_len);
			//return true;
		}

	}
}