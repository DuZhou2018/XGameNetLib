/***************************************************************************************************************************************
	author:	    DuZhou   qq: 1265008170,   mail: zht213@163.com
	created:    2021/9/28 11:37
	filename:   TCPServer.hpp
	purpose:	
***************************************************************************************************************************************/
#ifndef __OMP_TCPSERVER_IMPL_HEADER__
#define __OMP_TCPSERVER_IMPL_HEADER__
#include "./TcpServerBase.h"
#include "./UdpServerBase.h"
#include "./TCPServerSession.hpp"
#include "./HttpServerSession.hpp"
#include "./WebSocketSession.hpp"
#include "./UDPServerSession.hpp"

namespace peony {
	namespace net {
		class TCPServer:public TcpServerBase<TCPServerSession>
		{
		public:
			explicit TCPServer( TNewServerParam &param,server_deallocator_type DelSelfFun);
			virtual ~TCPServer();

        public://fordebug
			virtual IHttpReq    *GetHttpReq( unsigned uConID ){ return 0;}
			virtual bool         IsTcpCon() { return true; }

		};

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        HttpServer           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		class HttpServer:public TcpServerBase<HttpServerSession>
		{
		public:
			explicit HttpServer( TNewServerParam &param,server_deallocator_type DelSelfFun);
			virtual ~HttpServer();

		public://fordebug
			virtual IHttpReq    *GetHttpReq(unsigned uConID);
			virtual bool         IsHttpServer(){ return true; }
		};

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@        WebSocketServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		class WebSocketServer:public TcpServerBase<WebSocketSession>
		{
		public:
			explicit WebSocketServer( TNewServerParam &param,server_deallocator_type DelSelfFun);
			virtual ~WebSocketServer();

		public://fordebug
			virtual IHttpReq    *GetHttpReq( unsigned uConID ){ return 0;}
			virtual bool         IsWssCon() { return true; }
			virtual int 		 SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText);
		};

        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@        UDPServer          @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        class UDPServer :public UdpServerBase<UDPServerSession>
        {
        public:
            explicit UDPServer(TNewServerParam &param, server_deallocator_type DelSelfFun);
            virtual ~UDPServer();

        public://fordebug
            virtual IHttpReq    *GetHttpReq(unsigned uConID) { return 0; }
            virtual bool         IsHttpServer() { return false; }
            virtual bool         IsWssCon() { return false; }
        };

	}	// end namespace net
}	// end namespace peony

#endif // end of #define __OMP_TCPSERVER_IMPL_HEADER__
