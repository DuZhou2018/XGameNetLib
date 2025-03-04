/*****************************************************************************************************************************
    Created:   2017/9/5
    Author:    ZhangHongtao,   1265008170@qq.com
    FileName:
 
 //Opcode  SetBackID = iMsgType
 // 	���ݰ����ͣ�frame type����ռ4bits
 // 	0x0����ʶһ���м����ݰ�
 // 	0x1����ʶһ��text�������ݰ�
 // 	0x2����ʶһ��binary�������ݰ�
 // 	0x3-7������
 // 	0x8����ʶһ���Ͽ������������ݰ�
 // 	0x9����ʶһ��ping�������ݰ�
 // 	0xA����ʾһ��pong�������ݰ�
 // 	0xB-F������
   
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
			//socket��һЩ�ص�
			enum XMarkWebSocket
			{
				XMWSocket_IsConOk   = 1<<21,	//������ӳɹ��ɹ�
			};

			enum XHandReadType
			{//��ζ�����ʲô����
				EHReadType_NoDef = 0,
				EHReadType_LogicConnect,		//�߼�����
				EHReadType_LogicMsgHeadFirst,	//��Ϣͷ�ĵ�һ����
				EHReadType_LogicMsgHeadSecond,  //��Ϣͷ�ĵڶ�����
				EHReadType_LogicMsgBody,        //��Ϣ��

			};

		private:
			struct TCurMsgHead
			{
				bool			IsMsgEof;			//���λ����������Ϣ�Ƿ����,���Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�
				int				iMsgType;			//�ж���Ϣ����
				bool			IsHaseMask;			//�ж��Ƿ������봦��
	            unsigned char	chMask[4];
				unsigned		uMsgLength;			//��Ϣ�ĳ���

				bool            HeadRecOver;        //��ͷ��������һ���ֺ͵ڶ������Ƿ񶼽��������
				bool            HearFirstIsRead;    //��ͷ�ĵ�һ�����Ƿ��Ѿ���ȡ��
				char			HeadBuffer[64];		//�������հ�ͷ����Ϣ,2���ֽڣ�4�����룬��������8���ֽڵĳ���
				unsigned        HeadSecondPartLen;	//��ͷ�ĵڶ����ֳ���
				char           *pBodyPosInRecBuf;   //�����ڽ��ջ���������Ŀ�ʼ��λ��

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
					//���λ����������Ϣ�Ƿ����,���Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�
					IsMsgEof = !!(chFirstByte>>7);
					//�ж���Ϣ����
					iMsgType = chFirstByte&(0XF);
					unsigned char chSecondByte = (unsigned char)HeadBuffer[1];
					//�ж��Ƿ������봦��
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
                �������߼���Ϣ���ղ���
			********************************************************************************/
			//����һ���߼���Ϣ��������Ҳ���ܶ����ǰ�ͷ��Ҳ�����ǰ���
			bool            BeginOnceReadLogicMsg();
			void            HandRead_LogicMsgHeadFirst( size_t bytes_transferred );
			void            HandRead_LogicMsgHeadSecond( size_t bytes_transferred );
			void            HandRead_LogicMsgBody( size_t bytes_transferred );


			/**********************************************************************************
				Created:   2017/9/7 	Author:    ZhangHongtao,   1265008170@qq.com
                ��������
			********************************************************************************/
			//����һ�����ȵ���Ϣ
			int             ImpRecMaxLenMsg( XHandReadType xReadType,char *pRecBuffer,unsigned uMaxLen, unsigned uMinLen );
			int             ImpRecMaxLenMsg_ReadBody();
			
			int             SocketSendOneMsg( char *pSend,unsigned uData_len );
			int				PushSendQueue( const void* data_ptr, size_t data_size );
			int             SendMsg_Aaa(const char *pdata,unsigned data_len,int iDataType );

            //����������Ӧ��Ϣ���ͻ���
            void         	SendPongMsgToCc(unsigned  connectid,INPROTOCOL::TInNiceNetHead *pMsgPing);

		protected:
			TCurMsgHead		m_MsgHead;			//��ǰ��Ϣ��ͷ
			char            m_ConnectBuf[1024]; //������������,���ӽ���������������Ϣ��ͷ��
        };
    }
}
#endif // end of #define __OMP_TCPCONNECTION_HEADER__
