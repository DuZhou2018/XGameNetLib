/********************************************************************
	created:	2:7:2010   16:29
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\INiceNet.h
	author:		duzhou
	
	purpose:	�ṩ�����������ֻ֧��tcp
	//AppMain                 1
	//applog                  1
	//netlog                  1
	//���繤���߳�              3
	//Asio��server�߳�         1

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
			���棺�����������ӿ���ɾ�����ӵ�ʱ�򣬲��ܱ�֤���е����ݶ����������!
			**************************************************************************/
            virtual bool			DelClient(unsigned uClientID,string strWhy) = 0;
            virtual bool			DelServer( unsigned uServerID ) = 0;
			virtual void			CloseConByID( unsigned uConID,string strWhy ) = 0;

            virtual bool			SendMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool ischeck=true)=0;
			virtual bool			SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText)=0;

			/*************************************************************************
			���棺 App�Զ�����Ҫ����1<<10, С�ڵ��ڲ��Ѿ�ʹ��EmSocketMarketBase
			**************************************************************************/
            virtual void            SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel ) = 0;
			virtual bool            IsXAttrib(unsigned conid,unsigned xadd) = 0;

			virtual bool            IsValidConid(unsigned conid)=0;             //�Ƿ�����������id
			virtual void            SetConData(unsigned conid,uint64_t udata)=0; //�ṩһ��������ʱ���ݵĽӿ�
			virtual uint64_t        GetConData(unsigned conid)=0;               //�ṩһ��������ʱ���ݵĽӿ�

			/*************************************************************************
			���棺�������ͼͨ����������õ��������ڼ�����Զ�̽ӿڣ����᷵��ʧ��!
			**************************************************************************/
			virtual string			GetRemoteIp(unsigned &uPort,unsigned uConnectid)=0;
			virtual string			GetRemoteIp(unsigned uConnectid) =0;
			virtual string			GetLocalIp(unsigned &uPort,unsigned uConnectid)=0;
			virtual string			GetLocalIp(unsigned uConnectid) =0;
			/********************************************************************
				created:	2009/11/12	11:06
				author:		zhanghongtao	
				purpose:	�õ�һ��������ʹ�õĻ���������Ϣ������Ǽ�����uConnectid����ʧ��!
			*********************************************************************/
			virtual bool			GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info) =0;
			virtual bool            AddCounterForCon(unsigned ucondid,string strdes)=0;
			virtual bool            DelCounterForCon(unsigned ucondid)=0;

			//��ȡһ�����ؼ���������, �õ���ǰ��������Ŀ
			virtual unsigned        GetMaxCounterCount(unsigned uConnectid)=0;
			virtual unsigned        GetSeverCurConnectCount(unsigned uConnectid)=0;


			/********************************************************************
			created:	2009/11/12	11:06
			author:		zhanghongtao	
			purpose:	��һ���������ӵ�֮���ӵõ�������������ip�Ͷ˿�!
			*********************************************************************/
			virtual string          GetServerIpPortBySubConID( unsigned subconid,unsigned &port )=0;

			/********************************************************************
			created:	2017/03/20	10:06
			author:		zhanghongtao
			purpose:	����߼���������λ��,
			*********************************************************************/
			virtual void            SetCurAppMainCallPos(string strCallPos,unsigned iMaxTmLen)=0;
			virtual string          CheckAppMainThreadIsLock() = 0;
			virtual string          GetCurThreadIdStr()=0;
        };
    }
}

#endif
