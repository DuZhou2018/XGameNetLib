/*****************************************************************************************************************************
    Created:   2017/9/5
    Author:    ZhangHongtao,   1265008170@qq.com
    FileName:
 
 //Opcode  SetBackID = iMsgType
 // 	数据包类型（frame type），占4bits
 // 	0x0：标识一个中间数据包
 // 	0x1：标识一个text类型数据包
 // 	0x2：标识一个binary类型数据包
 // 	0x3-7：保留
 // 	0x8：标识一个断开连接类型数据包
 // 	0x9：标识一个ping类型数据包
 // 	0xA：表示一个pong类型数据包
 // 	0xB-F：保留
   
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-------+-+-------------+-------------------------------+
   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
   | |1|2|3|       |K|             |                               |
   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
   |     Extended payload length continued, if payload len == 127  |
   + - - - - - - - - - - - - - - - +-------------------------------+
   |                               |Masking-key, if MASK set to 1  |
   +-------------------------------+-------------------------------+
   | Masking-key (continued)       |          Payload Data         |
   +-------------------------------- - - - - - - - - - - - - - - - +
   :                     Payload Data continued ...                :
   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
   |                     Payload Data continued ...                |
   +---------------------------------------------------------------+

   opcode:
   *  %x0 denotes a continuation frame
   *  %x1 denotes a text frame
   *  %x2 denotes a binary frame
   *  %x3-7 are reserved for further non-control frames
   *  %x8 denotes a connection close
   *  %x9 denotes a ping
   *  %xA denotes a pong
   *  %xB-F are reserved for further control frames

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

   Masking-key:  0 or 4 bytes

******************************************************************************************************************************/
#ifndef WSSocketBase_h__
#define WSSocketBase_h__
#include "./TCPXSessionBase.hpp"
namespace peony {
    namespace net {
        class CWSSocketBase : public XTcpSocketBase<CWSSocketBase>
        {
		protected:
			//socket的一些特点
			enum XMarkWebSocket
			{
				XMWSocket_IsConOk   = 1<<21,	//标记链接成功成功
			};

			enum XHandReadType
			{//这次读的是什么数据
				EHReadType_NoDef = 0,
				EHReadType_LogicConnect,		//逻辑链接
				EHReadType_LogicMsgHeadFirst,	//消息头的第一部分
				EHReadType_LogicMsgHeadSecond,  //消息头的第二部分
				EHReadType_LogicMsgBody,        //消息体

			};

		private:
			struct TCurMsgHead
			{
				bool			IsMsgEof;			//最高位用于描述消息是否结束,如果为1则该消息为消息尾部,如果为零则还有后续数据包
				int				iMsgType;			//判断消息类型
				bool			IsHaseMask;			//判断是否又掩码处理
	            unsigned char	chMask[4];
				unsigned		uMsgLength;			//消息的长度

				bool            HeadRecOver;        //包头，包括第一部分和第二部分是否都接收完成了
				bool            HearFirstIsRead;    //包头的第一部分是否已经读取了
				char			HeadBuffer[64];		//用来接收包头的信息,2个字节，4个掩码，还可能有8个字节的长度
				unsigned        HeadSecondPartLen;	//包头的第二部分长度
				char           *pBodyPosInRecBuf;   //数据在接收缓冲区里面的开始的位置

			public:
				void  Reset()
				{
					IsMsgEof	= false;
					iMsgType	= 0;
					IsHaseMask	= false;
					uMsgLength  = 0;
					memset(chMask,0,sizeof(chMask) );

					HeadRecOver       = false;
					HearFirstIsRead   = false;
					HeadSecondPartLen = 0;
					pBodyPosInRecBuf  = 0;
					memset(HeadBuffer,0,sizeof(HeadBuffer) );
				}
				void AnalyMsgHeadBuf()
				{
					unsigned char chFirstByte = (unsigned char)HeadBuffer[0];
					//最高位用于描述消息是否结束,如果为1则该消息为消息尾部,如果为零则还有后续数据包
					IsMsgEof = !!(chFirstByte>>7);
					//判断消息类型
					iMsgType = chFirstByte&(0XF);
					unsigned char chSecondByte = (unsigned char)HeadBuffer[1];
					//判断是否又掩码处理
					IsHaseMask = !!(chSecondByte>>7);
					if( IsHaseMask )
						HeadSecondPartLen = 4;
					uMsgLength = (unsigned)(chSecondByte & 0x7F);
				}
				string LogInfo()
				{
					unsigned char chFirstByte  =(unsigned char)HeadBuffer[0];
					unsigned char chSecondByte =(unsigned char)HeadBuffer[1];

					ostringstream strLog;
					strLog<<"[ chFirst="<<(int)chFirstByte<<", chSec="<<(int)chSecondByte<<"; iType="<<iMsgType<<"; len="<<uMsgLength<<"; IsEof="<<IsMsgEof<<"; IMask="<<IsHaseMask<<"]";
					if( IsHaseMask )
					{
						string strMask = boost::str(boost::format(" mask=(%d_%d_%d_%d);")%((int)chMask[0])%((int)chMask[1])%((int)chMask[2])%((int)chMask[3]) );
						strLog<<strMask;
					}
					return strLog.str().c_str();
				}
			};
        public:
            CWSSocketBase(io_context &io_service,TXZTBaseInitPm &initPM);
			virtual ~CWSSocketBase();

		public:
			virtual void	StartSocket(){}
			virtual bool    OnRecive(unsigned uConnID, const void *pData, size_t data_len) { return false; }

		public:
            bool			Run(unsigned uCount);
			int             SendMsg(const char *pdata,unsigned data_len);
			int 			SendWebSocketMsg( const void *pdata,unsigned data_len,bool IsText);

		protected:
			virtual bool    XxTcpBeginRecWork();
			virtual bool	DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len);
			virtual void	XBase_ReadCb( const ZBoostErrCode& error,size_t bytes_transferred,int iDataType );
			virtual void	XBase_WriteCb(const ZBoostErrCode& error,int iDataType);
			virtual int     HandRead_LogicConnect(size_t bytes_transferred) { return 0; }

		protected:
			/**********************************************************************************
				Created:   2017/9/7 	Author:    ZhangHongtao,   1265008170@qq.com
                下面是逻辑消息接收部分
			********************************************************************************/
			//发起一次逻辑消息读操作，也可能读的是包头，也可能是包体
			bool            BeginOnceReadLogicMsg();
			void            HandRead_LogicMsgHeadFirst( size_t bytes_transferred );
			void            HandRead_LogicMsgHeadSecond( size_t bytes_transferred );
			void            HandRead_LogicMsgBody( size_t bytes_transferred );


			/**********************************************************************************
				Created:   2017/9/7 	Author:    ZhangHongtao,   1265008170@qq.com
                基础部分
			********************************************************************************/
			//接收一定长度的消息
			int             ImpRecMaxLenMsg( XHandReadType xReadType,char *pRecBuffer,unsigned uMaxLen, unsigned uMinLen );
			int             ImpRecMaxLenMsg_ReadBody();
			
			int             SocketSendOneMsg( char *pSend,unsigned uData_len );
			int				PushSendQueue( const void* data_ptr, size_t data_size );
			int             SendMsg_Aaa(const char *pdata,unsigned data_len,int iDataType );

            //发送心跳响应消息给客户端
            void         	SendPongMsgToCc(unsigned  connectid,INPROTOCOL::TInNiceNetHead *pMsgPing);

		protected:
			TCurMsgHead		m_MsgHead;			//当前消息的头
			char            m_ConnectBuf[1024]; //用来建立链接,链接建立后用来接收消息的头部
        };
    }
}
#endif // end of #define __OMP_TCPCONNECTION_HEADER__
