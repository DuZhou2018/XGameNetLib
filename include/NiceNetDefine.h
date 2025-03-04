/********************************************************************
	created:	2:7:2010   16:30
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\NiceNetDefine.h
	author:		Zhanghongtao
	
*********************************************************************/

#ifndef _INICE_NET_DEFINE
#define _INICE_NET_DEFINE

#include "./NiceNetMsgHead.h"
#include "./INiceLog.h"
#include "./IHttpReq.h"

namespace peony
{
    namespace net
    {
        enum EmPNetSocketType
        {
            EmPNetSType_NoDef = 0,
            EmPNetSType_Tcp,
            EmPNetSType_Http,
            EmPNetSType_WebSocket,
            EmPNetSType_Udp,
        };
		//socket的一些特点
		enum EmSocketMarketBase
		{
			socketmk_log				= 1<<0,   //[out]
			socketmk_isclose_sendfail	= 1<<1,   //发送失败是否关闭网络连接
			socketmk_IsHeartTimeout     = 1<<2,   //是否心跳超时
			socketmk_IsOpenState        = 1<<3,   //是否是open状态，已经开始工作了,保障在监听状态不检查超时
			socketmk_NoCheckHeart       = 1<<4,   //[out]是否检查心跳超时,可以自定定义单个的链接

			socketmk_client				= 1<<5,   //TcpClient的socket类型
			socketmk_server_client		= 1<<6,   //TcpServer的socket类型
			socketmk_logic_iniserver	= 1<<7,   //逻辑服务器群之间的socket类型,只有和client之间的链接不是

			socketmk_tcpsession_indelq	= 1<<10,
			//大于20每个类内部适用
		};

        static const unsigned   INVALID_CONN_INDEX = 0xFFFFFFFE;

        //app interface
        class IAppClient
        {
        public:
            static const int sc_status_connectok     = 0;
            static const int sc_status_connectclose  = -1;
            static const int sc_status_connectfail   = -2;
			static const int sc_status_gethostfail   = -3; //域名解析失败
        public:
            virtual void OnConnect( unsigned uConnectID,int status,std::string info)=0;
            virtual bool OnRecive(  unsigned uConnectID,const void *pData,size_t data_len)=0;
        };

        //app interface
        class IAppServer
        {
        public:
            static const int  sc_server_status_ok		 = 0;
            static const int  sc_server_status_close	 = 1;
			static const int  sc_server_status_isopctool = 2;
        public:
            virtual void OnSubConnect(unsigned uServerID,unsigned uSubConID,int status,string strInfo)=0;
            virtual bool OnRecive(unsigned uSubConID,const void *pData,unsigned data_len)=0;
            virtual void ServerStatus(unsigned uServerID,unsigned status)=0;

			/************************************************************
			IHttpReq 这个对象在回调里面可以使用，但是请服务器不要保存这个对象，
					 这个对象随时有可能会被释放掉,
					 如果你在后面需要使用到这个对象，可以查询这个对象使用.GetHttpReq
					 接口查询这个对象就可以了，如果返回0表示这个对象已经被释放了.
			*************************************************************/
			virtual void OnRecHttp(unsigned uSubConID,IHttpReq *pReq ){};
        };

        //socket kind
        enum EmNetKind
        {
            NetKind_No,
            NetKind_TCP = 10,
            NetKind_UDP,
        };
        enum EmClientNetState
        {
            CNetState_ConNo = 0,
            CNetState_ConING,
            CNetState_ConOK,
            CNetState_ConFail,
        };

        struct TNiceNetParam
        {
            unsigned     iWorkThreadNum;
            unsigned     iLogGrade;
			char         strLogDir[256]; 
			char         strLogFileName[64]; 
			char         strReciveDataPath[256];
			bool         IsOpenMThreadCheck;
			bool         IsCheckPackHead;
			unsigned     iSameIpMaxConnectCt; //同一个ip的最大链接数目
            TNiceNetParam()
            {
                iWorkThreadNum      = 2;
                iLogGrade           = LOG_Default;//LOG_ALL_LOG;
				memset(strLogDir,0,sizeof(strLogDir) );
				memset(strLogFileName,0,sizeof(strLogFileName) );
				memset(strReciveDataPath,0,sizeof(strReciveDataPath) );
				IsOpenMThreadCheck  = false;
				IsCheckPackHead     = true;
				iSameIpMaxConnectCt = 100;
            }
        };

        //20210611 外包函数调用接口,一些函数没有必要在这个库里面去实现
        //double (*pf); (int)  //指向参数为int类型,返回值为double 类型的指针
        typedef string  (*pNetFunFace_UTF8ToGBK) (const char *sour, unsigned iSourLen);
        typedef string  (*pNetFunFace_GBKToUTF8) (const char *sour, unsigned iSourLen);

        struct TNiceNetExFunFace
        {
            pNetFunFace_UTF8ToGBK  funUTF8ToGBK;
            pNetFunFace_GBKToUTF8  funGBKToUTF8;
            TNiceNetExFunFace()
            {
                funUTF8ToGBK = funGBKToUTF8 = 0;
            }
        };

        struct TNewClientParam
        {
            int             iNetType;           //EmPNetSocketType
			char            strServerIp[64];
			char            strServerPort[10];
            unsigned        uSendBuffSize;		//1024*n
			unsigned        uReciveBuffSize;	//1024*n
            IAppClient     *pApp;
			bool            isOpenCounter;		//=true open counter
			unsigned        uHeartTmSecond;     //心跳检查,如果为0不检查

            TNewClientParam()
            {
				memset(strServerIp,0,sizeof(strServerIp) );
				memset(strServerPort,0,sizeof(strServerPort) );
				strcpy(strServerIp, "0.0.0.0");
				iNetType = 1;
                uSendBuffSize       = 1024*32;
				uReciveBuffSize     = 1024*32;
                pApp                = 0;
				isOpenCounter       = false;
				uHeartTmSecond      = 0;
            }
        };

        struct TNewServerParam
        {
			char        strServerIp[16];
            unsigned    uServerPort;
            unsigned    uMaxConnectCount;
			unsigned    uSendBuffSize;			//1024*n
			unsigned    uReciveBuffSize;		//1024*n
            IAppServer *pApp;
			unsigned    uOpenCounterCount;		//表示client的数目,实际数目为client＋1为总的统计
			bool        using_allocmg;			//是否使用底层提供的内存管理方式
			bool        isdisconnect_sendfail;	//发送失败的时候是否断开链接
			unsigned    uHeartTmSecond;         //心跳检查,如果为0不检查
        public:
            TNewServerParam()
            {
				memset(strServerIp,0,sizeof(strServerIp) );
                uServerPort				= 6000;
				uSendBuffSize			= 1024*32;
				uReciveBuffSize			= 1024*32;
                uMaxConnectCount		= 64;
                pApp					= 0;
				uOpenCounterCount		= 0;
				using_allocmg			= false;
				isdisconnect_sendfail	= false;
				uHeartTmSecond          = 0;
            };
        };

		struct TSessionBufferInfo
		{
			unsigned  uSendOKCount;
			unsigned  uSendFailCount;
			unsigned  uReciveCount;
			unsigned  uSendUsedPercent;
			unsigned  uReciveUsedPercent;
		};
		struct TStatData
		{
			struct TStatItemTm
			{
				unsigned	lastTm;
				unsigned	uValue;
				TStatItemTm(){ lastTm = 0; uValue = 0;}

				void ResetSecond( unsigned uCurTm )
				{
					if( uCurTm != lastTm ){	lastTm = uCurTm; uValue = 0;}
				}
				void ResetMin( unsigned uCurTm )
				{
					if( uCurTm-lastTm<60 )
						return;
					lastTm = uCurTm; uValue = 0;
				}

			};

			unsigned			CurConnectCount;       //cur connect count
			TStatItemTm         LastSecondCount;       //last second new connect count;
			TStatItemTm         LastMinuteCount;       //last minute new connect count;
			TStatItemTm         RecLastSecFlow;        //last second flow (k)
			TStatItemTm         RecLastMinFlow;
			TStatItemTm         SendLastSecFlow;       //last second flow (k)
			TStatItemTm         SendLastMinFlow;

			TStatData()
			{
				CurConnectCount = 0;
			}
			void Reset( unsigned uCurTm )
			{
				LastSecondCount.ResetSecond( uCurTm );
				RecLastSecFlow.ResetSecond( uCurTm );
				SendLastSecFlow.ResetSecond( uCurTm );

				LastMinuteCount.ResetMin( uCurTm );
				RecLastMinFlow.ResetMin( uCurTm );
				SendLastMinFlow.ResetMin( uCurTm );
			}
		};

        //内部协议
        namespace INPROTOCOL
        {
			//测试协议 3997698000 = 61000,2000 
			static const unsigned NiceNetP_CTOS_TEST                = 3997698000;
			static const unsigned NiceNetP_STOC_TEST_ACK            = NiceNetP_CTOS_TEST+1;
			static const unsigned NiceNetP_NETHEART_NTF             = NiceNetP_STOC_TEST_ACK+1; //定义网络层的心跳
			static const unsigned NiceNetP_IN_MSG_END               = 3997698020;

			#pragma pack(push)
			#pragma pack(push,1)

			//测试协议
			struct TNiceNetTestMsgReq : public TNiceNetMsgHead
			{
                TNiceNetTestMsgReq():TNiceNetMsgHead(NiceNetP_CTOS_TEST,sizeof(TNiceNetTestMsgReq)){}
				unsigned    uIndex;
				char        chDes[512];
			};
			struct TNiceNetTestMsgRep : public TNiceNetMsgHead
			{
                TNiceNetTestMsgRep():TNiceNetMsgHead(NiceNetP_STOC_TEST_ACK,sizeof(TNiceNetTestMsgRep)){}
				unsigned    uIndex;
				char        chDes[512];
			};

			struct TNiceNetTestMsgNtf : public TNiceNetMsgHead
			{
				TNiceNetTestMsgNtf():TNiceNetMsgHead(NiceNetP_NETHEART_NTF,sizeof(TNiceNetTestMsgNtf)){}
				unsigned    uIndex;
				int         iMustValue;
			};

		#pragma pack(pop)//INPROTOCOL
        }

    }
}

#endif//_INICE_NET_DEFINE
