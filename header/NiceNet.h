/****************************************************************************************************
	created:	2010-8-24   10:30
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\header\NiceNet.h
	author:		Zhanghongtao
	
	purpose:	
	_STLP_USE_STATIC_LIB,使用stlport静态库
	存在问题：[1] "ec.message()"当有这个对象的调用的时候程序会僵死。估计是内存溢出!，好像在stlport里面有.
				  这个问题我初步的估计是因为boost的库编译的时候部分模块没有使用stlport的string而造成的.
*****************************************************************************************************/
#ifndef NICE_NET_IMP
#define NICE_NET_IMP
#include "./ANetVersion.hpp"
#include "./ClientServerIDMG.h"
#include "../include/INiceNet.h"

namespace peony
{
    namespace net
    {
        class CNiceNetSysParam;
        typedef map<unsigned,INiceNetServer*>MAP_NETSERVER;
        class CNiceNet : public INiceNet
        {
            //must thread save
        protected:
            friend class INiceNet;
            CNiceNet(void);
            ~CNiceNet(void);
        public:
            static CNiceNet &Instance();

        public:
            virtual bool			Init(TNiceNetParam &param);
            virtual bool			InitExFunFace(TNiceNetExFunFace &param);
            virtual string          GetNetVersion(){ return string(Sc_PNetCurVersion); }

            virtual void			UnInit();
            virtual unsigned	    Run(unsigned uCount,unsigned cur_time, unsigned cur_timeSs,unsigned &doMsgCt);
			virtual int				AddAllocatee(unsigned ubuffersize,unsigned maxcount);

            virtual unsigned        AddClient(TNewClientParam &param);
            virtual unsigned        AddServer(TNewServerParam &param);
            virtual unsigned        AddUdpServer(TNewServerParam &param);
			virtual unsigned        AddHttpServer(TNewServerParam &param);
            virtual unsigned        AddWebSocketServer(TNewServerParam &param);
            virtual bool            IsHttpCon(unsigned uConID);
			virtual bool            IsWssCon(unsigned uConID);
			virtual bool            IsTcpCon(unsigned uConID);
			virtual IHttpReq       *GetHttpReq( unsigned uConID );

            virtual bool            DelClient(unsigned uClientID,string strWhy);
            virtual bool            DelServer( unsigned uServerID );
			virtual void            CloseConByID( unsigned uConID,string strWhy );
			virtual void            SetLogPutGrade( unsigned iSet );
			virtual unsigned        GetLogPutGrade();
			virtual void			RealTimeWriteLog(); //保存网络层log到文件

            virtual bool            SendMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool ischeck=true);
			virtual bool			SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText);
			virtual void            SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel );
			virtual bool            IsXAttrib(unsigned conid,unsigned xmark);

			virtual bool            IsValidConid(unsigned conid);
			virtual void            SetConData(unsigned conid,uint64_t udata);
			virtual uint64_t        GetConData(unsigned conid);

			virtual string			GetRemoteIp(unsigned &uPort,unsigned uConnectid);
			virtual string			GetRemoteIp(unsigned uConnectid);
			virtual string			GetLocalIp(unsigned &uPort,unsigned uConnectid);
			virtual string			GetLocalIp(unsigned uConnectid);
			virtual bool            GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info);
			virtual bool            AddCounterForCon(unsigned ucondid,string strdes);
			virtual bool            DelCounterForCon(unsigned ucondid);
			virtual unsigned        GetMaxCounterCount(unsigned uConnectid);
			virtual unsigned        GetSeverCurConnectCount(unsigned uConnectid);
			virtual string          GetServerIpPortBySubConID( unsigned subconid,unsigned &port );
			virtual void            SetCurAppMainCallPos( string strCallPos,unsigned iMaxTmLen);
			virtual string          CheckAppMainThreadIsLock();//检查主线程是否死锁
			virtual string          GetCurThreadIdStr(){ return CInPubFun::GetThreadIdStr(); }

		public:
			INiceNetServer *        FindServer(unsigned serverinstanceid );
			class IFaceXClientMg *  FindXClientMg( unsigned iCcConID,EmIDKind IDKind=IDKIND_NODEF );
			void					OnTime_debug_serverthread();

        public:
            void                    LogoutCounter( class CNiceNetCounter *pReg_info );
			unsigned                GetNiceNetMark() {return m_NiceNetMark; }
			void                    SetNiceNetMark( unsigned mark ) { m_NiceNetMark=mark; }

		private:
			//是否是正确的ip地址
			bool                    is_right_ipstr( string strip );

        private:
            void                 	LogoutAllCounters();
            void                 	OnTime_1000Millisecond(unsigned cur_timeSs); //更新计数器
            void                 	SendMsg_CounterStateInfo();
            void                 	NoticeDelServer(INiceNetServer * pserver);
            void                 	PerformDelServer( unsigned uServerID );
			void                    OnTimeCheckHeartTimeout();
			void                    OnTimeCheckStopAccept();

        private:
			/********************************************************************
				created:	2009/11/16	16:11:2009   14:59
				author:		zhanghongtao	
				purpose:	保证APP层只能有一个线程调用
				m_RunMsgmutex;
			*********************************************************************/
			//boost::recursive_mutex &m_mutex;
            bool                    m_IsInitOk;
            MAP_NETSERVER           m_mapNetServer;
			unsigned                m_NiceNetMark;
			unsigned                m_LastCheckHeartTm;
			unsigned                m_LastCheckAcceptTm;

			TMainAppCallPos         m_MAppCallPos;
            TNiceNetExFunFace       m_NetExFunFace;
        };
    }
}
#endif

