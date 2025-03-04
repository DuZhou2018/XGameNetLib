#include "../header/WebSocketSession.hpp"
#include "../header/Scheduler.hpp"
#include "../header/CountersMg.hpp"
#include "../header/NiceNet.h"
#include "../header/CGlobal.hpp"
#include "../header/CountersMg.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/NetLogMg.h"
#include "../sha1/sha1.h"
#include "../include/NiceNetBase64.h"
#include "../header/InPublicPreDefine.hpp"

namespace peony
{
	namespace net 
	{
		WebSocketSession::WebSocketSession(io_context &io_service, TXZTBaseInitPm &initPM,const TNewServerParam &param )
		:CWSSocketBase(io_service,initPM ), m_param(param)
		{
		}

		WebSocketSession::~WebSocketSession()
		{
		}

		void WebSocketSession::StartSocket()
		{
 			OpenState();
 			if( 1 )
 			{
 				unsigned uRport;
 				string strRomeIP = this->GetRemoteIp( uRport );
 				unsigned uLocalport;
 				string strLocalIP = this->GetLocalIp( uLocalport );
 				ostringstream strselflog;
 				strselflog<<" [ServerSession:"<<GetConnectIndex()<<"] Local["<<strLocalIP<<","<<uLocalport<<"] Remote["<<strRomeIP<<","<<uRport<<"]";
 				if( strselflog.str().size()<sizeof(m_selflog) )
 				{
 					memset( m_selflog,0,sizeof(m_selflog) );
 					strcpy(m_selflog,strselflog.str().c_str() );
 				}
 				else
 				{
 					memset( m_selflog,0,sizeof(m_selflog) );
 					strcpy(m_selflog," HttpServerSession->start; " );
 				}
 				//NETLOG_NORMAL( m_selflog<<FUN_FILE_LINE );
 			}

		}

		bool WebSocketSession::OnRecive(unsigned uConnID, const void *pData, size_t data_len)
		{
			return m_param.pApp->OnRecive(uConnID, pData, (unsigned)data_len);
		}

		bool WebSocketSession::XxTcpBeginRecWork()
		{
			//启动读逻辑链接的的操作,最多读1000个,也许一次可以读完的,
			//服务器第一次要读取客户端的握手协议
			ImpRecMaxLenMsg(EHReadType_LogicConnect, m_ConnectBuf, sizeof(m_ConnectBuf) - 10, 2);
			return true;
		}
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		/*****************************************************************************************************************************
		Created:   2017/9/6
		Author:    ZhangHongtao,   1265008170@qq.com
		FileName:  G:\QzhXGame\PeonyNetWebSocket\source\CWSSocketBase.cpp
		下面的分段是处理握手协议

		GET / HTTP/1.1
		Host: 127.0.0.1:801
		Connection: Upgrade
		Pragma: no-cache
		Cache-Control: no-cache
		Upgrade: websocket
		Origin: http://www.blue-zero.com
		Sec-WebSocket-Version: 13
		User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36
		Accept-Encoding: gzip, deflate, sdch, br
		Accept-Language: zh-CN,zh;q=0.8,en;q=0.6
		Sec-WebSocket-Key: gfxI8FmiLtVmlYtf3J0G0g==
		Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits

		注意：作为一个例子，如果来自客户端握手请求中的 |Sec-WebSocket-Key| 的值是 dGhlIHNhbXBsZSBub25jZQ== 的话，
		那么服务端需要将 258EAFA5-E914-47DA-95CA-C5AB0DC85B11 字符串追加到其后，
		变成 dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11，
		对于哈希后的内容进行 base64 编码，最后得到 s3pPLMBiTxaQ9kYGzzhZRbK+xOo=，
		然后将这个值作为服务端返回的头字段 |Sec-WebSocket-Accept| 的字段值。

		******************************************************************************************************************************/
		//链接处理
		int  WebSocketSession::HandRead_LogicConnect(size_t bytes_transferred)
		{
			char strHBSplite[10] = "\r\n\r\n";
			//判断整个请求是否发送数据完成
			char *pReqBody = std::strstr(m_ConnectBuf, strHBSplite);
			if(0 == pReqBody)
				return 62302101;
			//NETLOG_SYSINFO(m_ConnectBuf << FUN_LINE);

			NVEC_STR vAllLine;
			CInPubFun::SpliteStrToA(vAllLine, m_ConnectBuf, "\r\n");
			if(vAllLine.size() < 1)
				return 62302103;

			string strSecWebsocketKey = "";
			char chName[128] = { 0 };
			for(NVEC_STR::iterator it = vAllLine.begin(); it != vAllLine.end(); it++)
			{
				char *pStrLine = (char*)it->c_str();
				if(strlen(pStrLine) > 1024)
				{
					NETLOG_ERROR(FUN_LINE << " [Ws链接头解析] [包头太长] " << pStrLine);
					continue;
				}
				//NETLOG_SYSINFO(pStrLine << FUN_LINE);

				char *pConLen = std::strstr(pStrLine, " ");
				if( 0 == pConLen )
				{
					continue;
				}

				if((0 == pConLen) || ((strlen(pConLen) + 2) > strlen(pStrLine)))
				{
					NETLOG_WARNING(FUN_LINE << " [Ws链接头解析] [逻辑分割错误] " << pStrLine);
					continue;
				}

				memset(chName, 0, sizeof(chName));
				if(strlen(pStrLine) - strlen(pConLen) + 10 > sizeof(chName))
				{
					NETLOG_WARNING(FUN_LINE << " [Ws链接头解析] [FieldName太长] " << pStrLine);
					continue;
				}
				memcpy(chName, pStrLine, strlen(pStrLine) - strlen(pConLen));
				pConLen += 1; //跳过空格第一个空格分隔符

				string strName = chName;
				string strData = pConLen;
				boost::trim(strName);
				boost::to_lower(strName);
				boost::trim(strData);

				if(string("get") == strName)
				{
				}
				else if(string("sec-websocket-key:") == strName)
				{
					strSecWebsocketKey = strData;
				}
			}

			HandRead_LogicConnect_DoShaKey(strSecWebsocketKey);
			return 0;
		}
		void WebSocketSession::HandRead_LogicConnect_DoShaKey(string strSecWebSocketKey)
		{
			//boost::replace_all( strSecWebSocketKey,"=","");
			string strTemp = strSecWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			//strTemp   = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			SHA1 tSha;
			tSha.addBytes(strTemp.c_str(), (unsigned)strTemp.size());

			char  chDesBuf[100] = { 0 };
			string strAaa = "";
			try {
				const unsigned char *pDes = tSha.getDigest();
				for(int t = 0; t < 20; t++)
				{
					unsigned char chTaa = *(pDes + t);
					string strWhy = boost::str(boost::format("%x") % (int)chTaa);
					strAaa += strWhy;
					strAaa += "; ";
				}
				Base64::Base64_Encode(chDesBuf, (const char*)pDes, 20);
			}
			catch(...) {
			}
			//NETLOG_SYSINFO("chDesBuf="<<chDesBuf<<"; strAaa="<<strAaa );
			string strret = chDesBuf;

			ostringstream strOsRep;
			strOsRep << "HTTP/1.1 101 Switching Protocols" << "\r\n";
			//strOsRep<<"Sec-WebSocket-Extensions: extension-list"<<"\r\n";
			//strOsRep<<"Sec-WebSocket-Version-Server: 1"<<"\r\n";
			//strOsRep<<"Server: ZhtWebSocketServer"<<"\r\n";
			//strOsRep<<"Sec-WebSocket-Protocol:chat"<<"\r\n";
			strOsRep << "Sec-WebSocket-Accept: " << strret << "\r\n";
			strOsRep << "Upgrade: websocket" << "\r\n";
			strOsRep << "Connection: Upgrade" << "\r\n";
			strOsRep << "Access-Control-Allow-Credentials: true" << "\r\n";
			strOsRep << "\r\n";


			CInPubFun::strcpy(m_ConnectBuf, sizeof(m_ConnectBuf), strOsRep.str().c_str(), 6542101);
			//NETLOG_SYSINFO("[ConnectRep]\r\n" << m_ConnectBuf << FUN_LINE);
			HandRead_LogicConnect_SendShaKeyRep();
		}
		int  WebSocketSession::HandRead_LogicConnect_SendShaKeyRep()
		{
			char *pHttpSend = m_ConnectBuf;
			unsigned uData_len = (unsigned)strlen(m_ConnectBuf);
			int iErrID = this->XBase_WriteData(pHttpSend, uData_len, uData_len, 0);
			if(iErrID > 0)
			{
				NETLOG_ERROR("[异常, net层]. wirte<1>! " << LogSelf() << FUN_LINE);
				PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog);
				CloseSocketOut("SendMsgException1!");
				return 5210101;
			}
			this->SetXMark(XMWSocket_IsConOk);
			return 0;
		}

	}
}
