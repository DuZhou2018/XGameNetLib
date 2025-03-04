/********************************************************************
created:	2:7:2010   16:30
filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\INiceNetServer.h
author:		Zhanghongtao

purpose:	�ṩ�������ӿ�
*********************************************************************/

#ifndef _INICE_NET_SERVER
#define _INICE_NET_SERVER
#include "../include/NiceNetDefine.h"
#include "./InPublicPreDefine.hpp"
#include "./RSBuffer.hpp"
#include "./TCPPakDef.hpp"

namespace peony
{
	namespace net
	{
		typedef tcp::socket TcpSocket;
		typedef udp::socket UdpSocket;
		typedef CReciveBuffer<tcp_pak_header>RecvBufferType;
		typedef CSendBuffer<tcp_pak_header>  SendBufferType;

		typedef boost::shared_ptr<class INiceNetSocket> INetSocketPtr;
        typedef boost::shared_ptr<class INiceNetServer> XServerSPtr;
        typedef boost::function< void(unsigned, unsigned, EmConnectInfo, std::string) >hand_onconnect;

		bool tcpserver_in_alloc_buffer(bool using_alloc_sys,char *&buffer,unsigned usize,bool is_req_alloc,unsigned umark);

		//��Щ�¼��ŵ���ѭ������ȥ������Ϊ�̰߳�ȫ��һЩ��Ҫ
		void mainrun_call_newtcpsocket(INiceNetServer *pself,INetSocketPtr session_ptr,bool isok);
		void mainrun_call_onrecvmsg(INetSocketPtr pAutoPtrSession );
		void mainrun_call_onconnected(unsigned uServerID,unsigned connindex,EmConnectInfo status,std::string info,INetSocketPtr pAutoPtrSession);

		class INiceNetSocket : public CPNetMark,private boost::noncopyable,public boost::enable_shared_from_this<INiceNetSocket>
		{//CPNetMark=EmSocketMarketBase
		public:
			enum EmSockType
			{
				ESockType_NoDef  = 0,
				ESockType_Server_Tcp,
				ESockType_Server_Http,
				ESockType_Server_Wss,
                ESockType_Server_Udp,

				ESockType_Client = 10,
			};
		public:
			INiceNetSocket()
			{
				m_uAPPEData  = 0;
				m_LastLiveTm = PNGB::m_server_curTm;
				memset( m_selflog, 0, sizeof(m_selflog) );
				strcpy(m_selflog, "INiceNetSocket_Default...");
			}
		protected:
			virtual ~INiceNetSocket(){}

		public:
			virtual void		StartSocket(){}
			virtual bool		Run(unsigned uCount){ return false; }
			virtual void		CloseSocketOut(string strwhy){}
			virtual unsigned	GetConnectIndex(){ return m_conn_index; }
			virtual TcpSocket*  GetTcpSocket() { return 0; }
			virtual UdpSocket*  GetUdpSocket() { return 0; }
			virtual bool		IsGoodConnect(){ return true; }

			virtual string		GetRemoteIp(unsigned &uPort) { return ""; }
            virtual string		GetRemoteIp(){ return ""; }
            virtual string		GetLocalIp(unsigned &uPort){ return ""; }
            virtual string		GetLocalIp(){ return ""; }

            virtual void        XBase_UdpReadCb( const ZBoostErrCode& error, std::size_t bytes_transferred){}
            virtual void		XBase_UdpWriteCb(const ZBoostErrCode& error, std::size_t bytes_transferred){}
            virtual void		XBase_ReadCb( const ZBoostErrCode& error, size_t bytes_transferred, int iDataType){}
			virtual void		XBase_WriteCb(const ZBoostErrCode& error, int iDataType){}

			virtual void		handle_resolve_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator){}
			virtual void		handle_connect_zhtcc(const ZBoostErrCode& err,tcp::resolver::iterator endpoint_iterator){}
            virtual void		handle_udp_resolve_zhtcc(const ZBoostErrCode& err, udp::resolver::iterator endpoint_iterator) {}
            virtual void		handle_udp_connect_zhtcc(const ZBoostErrCode& err, udp::resolver::iterator endpoint_iterator) {}
			virtual int			SubCheckUdpIsMaySendMsg(SendBufferType &send_buf, const void* data_ptr, unsigned data_size, bool &IsCloseSocket);
			virtual int			SubCheckTcpIsMaySendMsg(SendBufferType &send_buf, const void* data_ptr, unsigned data_size, bool &IsCloseSocket);

            void		        OnConnect_CallXApp(unsigned uMyServerID, unsigned uConnectID, int status, std::string info);
            bool                SockType_IsClient() { return ESockType_Client==m_SockType; }
            void		        SetConData(uint64_t udata) { m_uAPPEData=udata; }
            uint64_t	        GetConData() { return m_uAPPEData; }
            const char *        LogSelf() { return m_selflog; }
            void			    GetNewConID();
            void                FreeConID();
            void                SetMyXServerPtr(XServerSPtr pAutoParent) { m_pMyXServer=pAutoParent; }
            bool                CheckHeartTimeout(unsigned uMaxTime);

		protected:
			void				zht_CallMRecvHandler();
			tcp_pak_header *    SendNextCheckSendMsgHead(SendBufferType &send_buf, bool &IsHasErr);
			int					SubCheckSendMsgHead(SendBufferType &send_buf);
			void                SubFunBuildSelfLog();

        protected:
			EmSockType			m_SockType;				//��������
			bool                m_IsConnectOk;			//�Ƿ����ӳɹ�
			unsigned			m_conn_index;			//����ID
			unsigned            m_LastLiveTm;			//����յ���Ϣ�����߳ɹ�������Ϣ��ʱ��;
			unsigned			m_uXServerID;			//�ҵĹ���
			XServerSPtr         m_pMyXServer;			//�����ҵĶ��󣬱�֤����֮ǰ��������

			char                m_RomeIp[20];			//�Է���IP��ַ
			char                m_selflog[128];			//�Լ�id�ȹ̶���log��Ϣ
			char                m_SocketTypeName[20];	//Tcp,Http,WebSocket
			char                m_SocketWhyClose[64];	//��¼Ϊʲô�ر�������ӣ�����Ƿ����������رյ�ǰ׺��Me,�Է��رյ���ohter 

			TStatData          *m_pStatData;            //ͳ������
			uint64_t            m_uAPPEData;			//Ϊapp���ṩһ���������ݵĽӿ�
            hand_onconnect		m_InFunConnect;			//֪ͨApp�����¼��Ļص��ӿ�

        };


		class INiceNetServer : private boost::noncopyable,public boost::enable_shared_from_this<INiceNetServer>
		{
		public:
			struct TCurListenCon
			{
				//��ʾ�����Ƿ��ڽ��������ӣ���Ϊ��������Ŀ�ﵽ����ʱ��,listen��ֹͣ��������Ҫ��ʱȥ���
				//���Ҵ��������
				bool		isStop;			//��ֹͣ����״̬
				unsigned	uStopTm;		//��ʼ��ɾ����ʱ��
				unsigned    uDelBeginTm;	//��ʼ��ɾ����ʱ�䣬������¼���ʼ5���ſ���ɾ��

				TCurListenCon(){ isStop=false; uStopTm=0; uDelBeginTm=0; }
			};
		protected:
			INiceNetServer();
		public:
			virtual ~INiceNetServer();

		public:
			virtual int 		 SendMsg(const unsigned &uConnectID, const void *pData,unsigned uLength)=0;
			virtual int 		 SendWebSocketMsg(unsigned uConnectid,const void *pdata,unsigned data_len,bool IsText){ return 88801122; }
			virtual bool		 CloseSub( unsigned conn_index,string strWhy )=0;
			virtual unsigned     GetID(){ return m_instance_id; }
			virtual void		 stop( bool wait_until_finished /*= true */ )=0;

			virtual string       GetListenIpPort()=0;
			virtual string       GetRemoteIp(unsigned &uPort,unsigned conn_index)=0;
			virtual string       GetLocalIp(unsigned &uPort,unsigned conn_index)=0;
			virtual bool         Server_GetSessionBufferInfo(unsigned uConnectid,TSessionBufferInfo &info)=0;
			virtual bool         Server_ClearSeeionBufferInfo(unsigned uConnectid)=0;
			virtual unsigned     GetMaxCounterCount()=0;
			virtual unsigned     GetSeverCurConnectCount()=0;
			virtual unsigned     get_conn_count( void )=0;
			virtual void         SetXAttrib( unsigned conid,unsigned xadd, unsigned xdel )=0;
			virtual bool         IsXAttrib(unsigned conid,unsigned xmark)=0;
			virtual void         SetConData(unsigned conid,uint64_t udata)=0;
			virtual uint64_t     GetConData(unsigned conid)=0;
			virtual IHttpReq    *GetHttpReq( unsigned uConID )=0;
			virtual IAppServer  *GetIAppServer()=0;
			virtual bool         IsHttpServer(){ return false; }
			virtual bool         IsWssCon(){ return false; }
			virtual bool         IsTcpCon(){ return false; }
			virtual void         OnTimeCheckHeartTimeout( std::list<unsigned> &listConID)=0;
			virtual int          GetCurSessionsPoolSize()=0; //��ǰ���ӳص���Ŀ
			virtual void         CheckIsStopAccept()=0; 
			virtual bool         PNetListen()=0;
			virtual int          is_may_accept_newconnect( string strRemoteIp )=0;
            virtual void		 SendOne_WriteDataCb(const ZBoostErrCode& error, std::size_t iDataType) {}

			TStatData *			 GetStatData(){ return &m_StatData; }
			unsigned             GetDelStartTm(){ return m_ListenCon.uDelBeginTm; }

		protected:
			//ͳ������
			void                 StatOneNewConnect(){ m_StatData.LastMinuteCount.uValue+=1; m_StatData.LastSecondCount.uValue+=1; }

		protected:
			TStatData				m_StatData;
			TCurListenCon			m_ListenCon;    //����״̬����

			io_context &			m_io_service;
			AsioStrand &			m_strand;
			unsigned    		    m_instance_id;

		};

        typedef boost::function< void(unsigned) >onclose_zht_handler;
        typedef boost::function< void(INetSocketPtr)>onrecv_zht_handler;
        typedef boost::function< void(unsigned, IHttpReq*) >onrecv_http_handler;
        typedef boost::function< void(unsigned, EmConnectInfo, std::string, INetSocketPtr) >onconnect_zht_handler;

        struct TXZTBaseInitPm
        {
            unsigned          mXServerID; //�ҹ����Ǹ��ϰ�
            TStatData		 *pStatData;
            mghandler_buffer  r_alloc;
            mghandler_buffer  s_alloc;
            unsigned          r_size;
            unsigned          s_size;
            hand_onconnect    mFunConnect;
            TXZTBaseInitPm()
            {
                mXServerID  = 0;
                pStatData   = 0;
                r_alloc     = 0;
                s_alloc     = 0;
                r_size      = 0;
                s_size      = 0;
                mFunConnect = 0;
            }
        };
	}
}
#endif
