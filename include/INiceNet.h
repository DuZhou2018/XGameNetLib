/********************************************************************
	created:	2:7:2010   16:29
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\INiceNet.h
	author:		duzhou
	
	purpose:	提供网络服务器，只支持tcp
	//AppMain                 1
	//applog                  1
	//netlog                  1
	//网络工作线程              3
	//Asio的server线程         1

*********************************************************************/
#ifndef _INICE_NET
#define _INICE_NET

#include "INiceNetCounter.h"

#ifndef WIN32
	#include "boost/cstdint.hpp"
	typedef boost::uint64_t  uint64_t;
#endif
namespace peony
{
    namespace net
    {
        class INiceNet
        {
        public:
            static INiceNet &Instance();
        protected:
            INiceNet(){}
            virtual ~INiceNet(){}
        public:
            virtual bool			Init(TNiceNetParam &param) = 0;
            virtual bool			InitExFunFace(TNiceNetExFunFace &param) = 0;
            virtual string          GetNetVersion()=0;

            virtual void			UnInit() = 0;
			virtual unsigned	    Run(unsigned uCount,unsigned cur_time, unsigned cur_timeSs,unsigned &doMsgCt)=0;
			virtual int				AddAllocatee(unsigned ubuffersize,unsigned maxcount) = 0;

            virtual unsigned        AddClient(TNewClientParam &newClient) = 0;
            virtual unsigned        AddServer(TNewServerParam &newServer) = 0;
            virtual unsigned        AddUdpServer(TNewServerParam &newServer) = 0;
            virtual unsigned        AddHttpServer(TNewServerParam &newServer)= 0;
			virtual bool            IsHttpCon(unsigned uConID) = 0;
			virtual bool            IsWssCon(unsigned uConID) = 0;
			virtual bool            IsTcpCon(unsigned uConID) = 0;
			virtual IHttpReq       *GetHttpReq( unsigned uConID )=0;
			virtual unsigned        AddWebSocketServer(TNewServerParam &param)=0;

			virtual void            SetLogPutGrade( unsigned iSet ) = 0;
			virtual unsigned        GetLogPutGrade() = 0;
			virtual void			RealTimeWriteLog()=0;
			/*************************************************************************
			警告：当你调用这个接口来删除连接的时候，不能保证所有的数据都被发送完成!
			**************************************************************************/
            virtual bool			DelClient(unsigned uClientID,string strWhy) = 0;
            virtual bool			DelServer( unsigned uServerID ) = 0;
			virtual void			CloseConByID( unsigned uConID,string strWhy ) = 0;

            virtual bool			SendMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool ischeck=true)=0;
			virtual bool			SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText)=0;

			/*************************************************************************
			警告： App自定义需要大于1<<10, 小于的内部已经使用EmSocketMarketBase
			**************************************************************************/
            virtual void            SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel ) = 0;
			virtual bool            IsXAttrib(unsigned conid,unsigned xadd) = 0;

			virtual bool            IsValidConid(unsigned conid)=0;             //是否存在这个链接id
			virtual void            SetConData(unsigned conid,uint64_t udata)=0; //提供一个保存临时数据的接口
			virtual uint64_t        GetConData(unsigned conid)=0;               //提供一个保存临时数据的接口

			/*************************************************************************
			警告：如果你试图通过这个函数得到本地正在监听的远程接口，将会返回失败!
			**************************************************************************/
			virtual string			GetRemoteIp(unsigned &uPort,unsigned uConnectid)=0;
			virtual string			GetRemoteIp(unsigned uConnectid) =0;
			virtual string			GetLocalIp(unsigned &uPort,unsigned uConnectid)=0;
			virtual string			GetLocalIp(unsigned uConnectid) =0;
			/********************************************************************
				created:	2009/11/12	11:06
				author:		zhanghongtao	
				purpose:	得到一个连接所使用的缓冲区的信息，如果是监听的uConnectid返回失败!
			*********************************************************************/
			virtual bool			GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info) =0;
			virtual bool            AddCounterForCon(unsigned ucondid,string strdes)=0;
			virtual bool            DelCounterForCon(unsigned ucondid)=0;

			//获取一个本地监听的属性, 得到当前的连接数目
			virtual unsigned        GetMaxCounterCount(unsigned uConnectid)=0;
			virtual unsigned        GetSeverCurConnectCount(unsigned uConnectid)=0;


			/********************************************************************
			created:	2009/11/12	11:06
			author:		zhanghongtao	
			purpose:	从一个本地链接的之链接得到服务器监听的ip和端口!
			*********************************************************************/
			virtual string          GetServerIpPortBySubConID( unsigned subconid,unsigned &port )=0;

			/********************************************************************
			created:	2017/03/20	10:06
			author:		zhanghongtao
			purpose:	检查逻辑层死锁的位置,
			*********************************************************************/
			virtual void            SetCurAppMainCallPos(string strCallPos,unsigned iMaxTmLen)=0;
			virtual string          CheckAppMainThreadIsLock() = 0;
			virtual string          GetCurThreadIdStr()=0;
        };
    }
}

#endif
