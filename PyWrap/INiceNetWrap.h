#ifndef _INICE_NET_WRAP_FOR_PYTHON
#define _INICE_NET_WRAP_FOR_PYTHON

#if BOOST_VERSION>107200 
#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#endif

//copy $(TargetPath) "D:\python279\Lib\site-packages\peyone\peyonenet.pyd"
#include "../json/json.h"
#include "../header/CGlobal.hpp"
#include "../header/InPublicPreDefine.hpp"
#include "../include/INiceNet.h"
#include "../include/NiceNetBase64.h"
#include "../include/NiceNetMsgHead.h"
using namespace peony::net;
using namespace peony::net::INPROTOCOL;

#include <iostream>
#include <boost/python/object.hpp>


struct TcpPyWrapNetMsg : public TNiceNetMsgHead
{
public:
	TcpPyWrapNetMsg(unsigned opcode) : TNiceNetMsgHead(opcode, sizeof(TcpPyWrapNetMsg))
	{
		ZIndex  = 0;
		iBufLen = 0;
		memset( buffer, 0, sizeof(buffer) );
	}

public:
	int				iBufLen;
	unsigned char	buffer[1024*100];
};

struct IAppClientPy : IAppClient
{
public:
	virtual void OnConnect(unsigned int uConnectID, int status, std::string info){}
	virtual bool OnRecive(unsigned uConnectID, const void *pData, size_t data_len) { return true; }
	virtual bool OnReciveTcp(unsigned int uSubConID, unsigned iMsgID, const void *pData, unsigned int data_len) { return false; }
};
struct IAppClient_Wrap : IAppClientPy, wrapper<IAppClientPy>
{
    void OnConnect(unsigned int uConnectID,int status,std::string info)
    {
		try
		{
			boost::python::override oFunAa = this->get_override("OnConnect");
			if( oFunAa.is_none() )
				return;
			oFunAa(uConnectID,status,info);
		}catch(boost::python::error_already_set const&)
		{
			PyErr_Print();
			return;
		}catch(...)
		{
			return;
		}
    }
    bool OnRecive(unsigned uConnectID, const void *pData,size_t data_len)
    {
		//PYYLOG_SYSINFO("[收到消息 IAppClientPy]  data_len=" << data_len);
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();
        try
        {
			boost::python::override oFunAa = this->get_override("OnReciveTcp");
			if( oFunAa.is_none() )
				return false;

			const TcpPyWrapNetMsg *pMsgHead = (const TcpPyWrapNetMsg*)pData;
			boost::python::str in_data((char*)(pMsgHead->buffer), pMsgHead->iBufLen);
            return oFunAa(uConnectID, pMsgHead->GetMsgID(),in_data, pMsgHead->iBufLen );
        }
        catch (const boost::python::error_already_set)
        {
            PyErr_Print();
        }
        catch (std::exception* e)
        {
            std::cerr << e->what();
        }

        PyGILState_Release(gstate);
        return false;
    }
};

struct IAppServer_Wrap : IAppServer, wrapper<IAppServer>
{
	bool OnReciveTcp(unsigned int uSubConID, unsigned iMsgID, const void *pData, unsigned int data_len)
	{
		return true;
	}
	bool OnReciveWebSocket(unsigned int uSubConID, const void *pData, unsigned int data_len)
	{
		return true;
	}
	bool OnRecive(unsigned int uSubConID,const void *pData,unsigned int data_len)
	{
		bool isOk = false;
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure();
		bool IsWssSess = INiceNet::Instance().IsWssCon(uSubConID);
		//NETLOG__SYSINFO("[收到消息]... MsgID="<<0<<", IsWssSess="<<IsWssSess << ", data_len=" << data_len);
		try
		{
			isOk = true;
			boost::python::override oFunAa = this->get_override("OnReciveTcp");
			if(IsWssSess)
			{
				oFunAa = this->get_override("OnReciveWebSocket");
			}else{
				oFunAa = this->get_override("OnReciveTcp");
			}
			if( oFunAa.is_none() )
				return false;


			if( IsWssSess )
			{
				char *pMsgBody = (char *)pData + sizeof(TNiceNetMsgHead);
				unsigned iMsgBodyLen = data_len - sizeof(TNiceNetMsgHead);
				//NETLOG__SYSINFO("[收到ws消息]... MsgID=" << 0 << ", pMsgBody=" << pMsgBody << ", iMsgBodyLen=" << iMsgBodyLen);
				boost::python::str in_data(pMsgBody, iMsgBodyLen);
				oFunAa(uSubConID, in_data, iMsgBodyLen);
			}
			else {
				const TcpPyWrapNetMsg *pMsgHead = (const TcpPyWrapNetMsg*)pData;
				boost::python::str in_data((char*)(pMsgHead->buffer), pMsgHead->iBufLen);
				oFunAa(uSubConID,pMsgHead->GetMsgID(), in_data, pMsgHead->iBufLen);
			}

		}
		catch (const boost::python::error_already_set)
		{
			isOk = false;
			PyErr_Print();
		}
		catch(std::exception* e)
		{
			isOk = false;
			std::cerr << e->what();
		}

		PyGILState_Release(gstate);
		return isOk;
	}
	void OnRecHttp(unsigned int uSubConID,IHttpReq *pReq )
	{
		//ostringstream strlog;

		Json::Value jsHttpPm;
		if( pReq->IsGet() )
			jsHttpPm["ReqType"]="Get";
		else
			jsHttpPm["ReqType"]="Post";

		int iParamCt = pReq->GetParamCount();
		jsHttpPm["iParamCt"] = iParamCt;
		//strlog<<"[http iParamCt]="<<iParamCt;
		if( iParamCt>0 )
		{//get 方式
			for( int t=0;t<iParamCt; t++ )
			{
				string strAKey;
				string strAV	  = pReq->GetParamByIndex(t,strAKey);
				jsHttpPm[strAKey] = strAV;
			}
		}else
		{//post 方式
			static unsigned  sc_postmaxdatalen = 1024000;
			static char     *pBuffHttpDecode   = new char[sc_postmaxdatalen];
			unsigned         uBinLen           = 0;
			const char      *pBinData          = pReq->GetPostData( uBinLen );

			//strlog<<"; uBinLen="<<uBinLen;

			if( uBinLen*2>sc_postmaxdatalen )
			{
				jsHttpPm["reqBData"] = "Error ,post is toobig( max 512k )";
			}else{
				memset(pBuffHttpDecode,0,sc_postmaxdatalen);
				size_t uDesLencc     = Base64::Base64_Decode( pBuffHttpDecode,pBinData,uBinLen );
				jsHttpPm["reqBData"] = pBuffHttpDecode;
				//strlog<<";base64Len="<<uDesLencc;
			}
		}
		//printf( "INiceNetWrap::OnRecHttp: %s",strlog.str().c_str() );
		 
		Json::FastWriter json_writer;
		string  strJsonObj = json_writer.write(jsHttpPm);
		unsigned uDataLen  = (unsigned)strJsonObj.size();

		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure();
		try
		{
			boost::python::override oFunAa = this->get_override("OnHttpReq");
			if( oFunAa.is_none() )
				return;

			boost::python::str in_data((char*)strJsonObj.c_str(),uDataLen );
			oFunAa(uSubConID,in_data,uDataLen);
		}
		catch (const boost::python::error_already_set)
		{
			PyErr_Print();
		}
		catch(std::exception* e)
		{
			std::cerr << e->what();
		}

		PyGILState_Release(gstate);
	}
	void ServerStatus(unsigned int uServerID,unsigned int status)
	{
		this->get_override("ServerStatus")(status);
	}
	void OnSubConnect(unsigned int uServerID,unsigned int uSubConID,int status,string strInfo)
	{
		try
		{
			boost::python::override oFunAa = this->get_override("OnSubConnect");
			if( oFunAa.is_none() )
				return;

			oFunAa(uServerID,uSubConID,status);
		}
		catch (const boost::python::error_already_set)
		{
			PyErr_Print();
		}
		catch(std::exception* e)
		{
			std::cerr << e->what();
		}
	}
};

//网络层使用
struct INiceNet_Wrap
{
    INiceNet_Wrap()
    {
        INiceNet::Instance();
    }
	// jsParam = {"ThreadCount":3,"LogGrade":5,"LogDir":"D:\\PNetPyExLog","LogFileName":"TestPyEx" }
    int Init( const char *strJsParam )
    {
		if( 0 == strJsParam)
			return 989701;
		Json::Value  jsInit;
		Json::Reader jsReader;
		if (!jsReader.parse(strJsParam, jsInit))
		{
			return 989703;
		}


        TNiceNetParam param;
		param.IsCheckPackHead = false;
        param.iLogGrade = jsInit["LogGrade"].asInt();
		if( 0 == param.iLogGrade ){
			param.iLogGrade = LOG_Default;
		}
        param.iWorkThreadNum = jsInit["ThreadCount"].asInt();

		string strLogDir = jsInit["LogDir"].asString();
		if( strLogDir.size() < sizeof(param.strLogDir) ){
			memcpy(param.strLogDir, strLogDir.c_str(), strLogDir.size());
		}
		string strFileName = jsInit["LogFileName"].asString();
		if(strFileName.size() < sizeof(param.strLogDir) ) {
			memcpy(param.strLogFileName, strFileName.c_str(), strFileName.size());
		}

		if( INiceNet::Instance().Init(param) )
			return 0;
		return 989729;
	}
    void UnInit()
    {
        INiceNet::Instance().UnInit();
    }
    bool Run(unsigned int uCount,unsigned int uCountSecond)
    {
		unsigned uCurTime = CInPubFun::GetNowTime();
		unsigned uSS = CInPubFun::GetNowTimeSs();
        return INiceNet::Instance().Run(uCount,uCurTime,uSS,uCountSecond);
    }

	void PutOneLog( int iLogGrade, const char*pData )
	{//1,4=iLogGrade
		if (0 == pData)
			return;

		if( Log_Error == iLogGrade ){
			PYYLOG_ERROR( pData );
		}
		else {
			PYYLOG_SYSINFO(pData);
		}
	}
	void SaveLog()
	{
        NETLOG_SAVELOG();
	}
    unsigned AddClient(string strServerIp,string strServerPort,unsigned uRSBuffSize,IAppClient_Wrap &pApp)
    {
        TNewClientParam newClient;
		if( strServerIp.size()<sizeof(newClient.strServerIp) )
			memcpy( newClient.strServerIp,strServerIp.c_str(),strServerIp.size() );

		if( strServerPort.size()<sizeof(newClient.strServerPort) )
			memcpy( newClient.strServerPort,strServerPort.c_str(),strServerPort.size() );

        newClient.uReciveBuffSize   = uRSBuffSize;
		newClient.uSendBuffSize   = uRSBuffSize;
        newClient.pApp          = dynamic_cast<IAppClient *>(&pApp);
        assert(newClient.pApp);

        unsigned uClientID = INiceNet::Instance().AddClient( newClient );
		return uClientID;
    }
    
    unsigned AddServer(string strServerIp,unsigned int uServerPort,
                    unsigned int uMaxConnectCount,unsigned int uRSBuffSize,IAppServer_Wrap &pApp)
    {
        TNewServerParam newServer;
		if( strServerIp.size()<sizeof(newServer.strServerIp) )
			memcpy( newServer.strServerIp,strServerIp.c_str(),strServerIp.size() );

        newServer.uServerPort       = uServerPort;
        newServer.uMaxConnectCount  = uMaxConnectCount;
        newServer.uSendBuffSize     = uRSBuffSize;
		newServer.uReciveBuffSize   = uRSBuffSize;
        newServer.pApp  = dynamic_cast<IAppServer *>(&pApp);
        assert(newServer.pApp);
        
        return INiceNet::Instance().AddServer( newServer );
    }
	unsigned AddHttpServer(char* strServerIp,unsigned int uServerPort,
				unsigned int uMaxConnectCount,unsigned int uRSBuffSize,IAppServer_Wrap &pApp)
	{
		TNewServerParam newServer;
		if( strlen(strServerIp)<sizeof(newServer.strServerIp) )
			memcpy( newServer.strServerIp,strServerIp,strlen(strServerIp) );

		newServer.uServerPort       = uServerPort;
		newServer.uMaxConnectCount  = uMaxConnectCount;
		newServer.uSendBuffSize     = uRSBuffSize;
		newServer.uReciveBuffSize   = uRSBuffSize;
		newServer.pApp              = dynamic_cast<IAppServer *>(&pApp);
		assert(newServer.pApp);

		return INiceNet::Instance().AddHttpServer( newServer );
	}
	unsigned AddWebSocketServer(char* strServerIp, unsigned int uServerPort,
		unsigned int uMaxConnectCount, unsigned int uRSBuffSize, IAppServer_Wrap &pApp)
	{
		TNewServerParam newServer;
		if (strlen(strServerIp) < sizeof(newServer.strServerIp))
			memcpy(newServer.strServerIp, strServerIp, strlen(strServerIp));

		newServer.uServerPort      = uServerPort;
		newServer.uMaxConnectCount = uMaxConnectCount;
		newServer.uSendBuffSize    = uRSBuffSize;
		newServer.uReciveBuffSize  = uRSBuffSize;
		newServer.pApp = dynamic_cast<IAppServer *>(&pApp);
		assert(newServer.pApp);

		return INiceNet::Instance().AddWebSocketServer(newServer);
	}
	bool DelClient( unsigned iConnectID )
    {
        return INiceNet::Instance().DelClient( iConnectID,"PyDelCCc" );
        return false;
    }
	int HttpRepMsg( unsigned uConID,const char*pData,int iDataLen )
	{
		IHttpReq *pHttp = INiceNet::Instance().GetHttpReq( uConID );
		if( !pHttp )
			return 8302301;
		return pHttp->RepMsg( pData,iDataLen );
	}

	bool SendWebSocketMsg(unsigned uConID, const char*pData, int iDataLen,bool IsText )
	{
		//virtual bool SendWebSocketMsg(unsigned uConnectid, const void *pdata, unsigned data_len, bool IsText) = 0;
		return INiceNet::Instance().SendWebSocketMsg(uConID, pData,iDataLen, IsText );
	}
	bool SendTcpMsg(unsigned uConID, unsigned iMsgID, const char*pData, int iDataLen)
	{
		if( strlen(pData) != iDataLen )
		{
			PYYLOG_ERROR("[SendTcpMsg....fail..] 消息长度不匹配....... iDataLen="<< iDataLen<<", pDataLen="<<strlen(pData) );
			return false;
		}

		TcpPyWrapNetMsg tcpMsg(iMsgID);
		if( iDataLen + 4 > sizeof(tcpMsg.buffer))
		{
			PYYLOG_ERROR("[SendTcpMsg....fail..] 消息长度太大....... iDataLen=" << iDataLen << ", pDataLen=" << strlen(pData));
			return false;
		}
		memcpy(tcpMsg.buffer, pData, iDataLen);
		tcpMsg.iBufLen = iDataLen;
		unsigned iMsgLen = sizeof(tcpMsg) - sizeof(tcpMsg.buffer) + iDataLen + 1;
		tcpMsg.SetLen(iMsgLen);
		INiceNet::Instance().SendMsg(uConID, &tcpMsg,tcpMsg.GetLen() );
		return true;
	}
    bool SendTcpMsgBb(unsigned uConID,const char*pData,int iDataLen )
    {
        TNiceNetMsgHead *pBaseMsg = (TNiceNetMsgHead *)pData;
        return INiceNet::Instance().SendMsg( uConID,pBaseMsg,pBaseMsg->GetLen() );
    }
};

#endif //_INICE_NET_WRAP_FOR_PYTHON