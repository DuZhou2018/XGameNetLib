/********************************************************************
	created:	11:11:2013   14:41
	author:		ZhangHongtao  email: 1265008170@qq.com
	
	purpose:	ͨ�õ�����Э��ͷ(xuehai)

	struct TNiceNetMsgHead
	{
		unsigned	    Len;        //����4 byte, ��Ϣ������, ������ͷ�ĳ���
		unsigned	    OPCode;     //����4 byte, ��Ϣ���
		unsigned	    CellID;     //����4 byte, �߼����ӳɹ����Ӧ��CellID
		unsigned short  CheckCode;  //����2 byte, У����,Ĭ�� 21301
		unsigned short  BackID;     //����2 byte, ����,Ĭ��Ϊ0
		unsigned        ZIndex;     //����4 byte, ��Ϣ�������,Ĭ��Ϊ0
	};

*********************************************************************/

#ifndef NiceNetMsgHead_h__
#define NiceNetMsgHead_h__
#include <memory.h>

namespace peony
{
    namespace net
    {
        //�ڲ�Э��
        namespace INPROTOCOL
        {
			const unsigned short sc_THttpCheckCodeIn = 21301;

			#pragma pack(push)
			#pragma pack(push,1)

			//BackID ��������
			#define DK_MAPPED		0x01			//ӳ������
			#define DK_WANGHUMSG    0x02			//������Ϣ���ṹ
			#define DK_COMPRESS		0x04			//ZIPѹ������

            struct TNiceNetMsgHead
            {
            public:
                TNiceNetMsgHead(unsigned msgID, unsigned msgLen) {
                    OPCode  = msgID;
                    Len     = msgLen; 
                    UserID  = BackID = ZIndex=0;
                    CheckCode = sc_THttpCheckCodeIn;
                }

                inline   void			SetLen(unsigned uLen) { this->Len = (unsigned)uLen; }
                inline   unsigned		GetLen()const { return this->Len; }
                inline   unsigned		GetMsgID()const { return OPCode; }
                inline   void			SetMsgID(unsigned v) { OPCode = v; }

            public:
                unsigned	    Len;        //������Ϣ������
                unsigned	    OPCode;     //�������  
                unsigned	    UserID;    //
                unsigned short  CheckCode;  //У����21301;
                unsigned short  BackID;     //����;
                unsigned        ZIndex;     //��Ϣ������
            };

			struct TInNiceNetHead : public TNiceNetMsgHead
			{
			public:
                TInNiceNetHead() :TNiceNetMsgHead(0,0){ }
                TInNiceNetHead(unsigned opcode ) :TNiceNetMsgHead(opcode,sizeof(TInNiceNetHead) ) { ; SetCheckCode(); ZIndex=0; }
                TInNiceNetHead(unsigned opcode, unsigned len) :TNiceNetMsgHead(opcode, len) { ; SetCheckCode(); ZIndex=0; }


            public:
                inline   int			getCheckCode() { return this->CheckCode; }
				inline   void			SetCheckCode(){ CheckCode = sc_THttpCheckCodeIn;}
                
                inline   void			SetWebosockDataType(short v) { BackID=v; }
                inline   unsigned		GetWebosockDataType()const { return BackID; }
                inline   void			SetUdpContextIndex(unsigned v) { ZIndex=v; }
                inline   unsigned   	GetUdpContextIndex() { return ZIndex; }

                inline   void			SetHttpReqIndex(unsigned v) { UserID=v; }
                inline   unsigned		GetHttpReqIndex()const { return UserID; }
                inline   void			SetNetLogReaderID(unsigned v) { UserID=v; }
			};

		   #pragma pack(pop)//INPROTOCOL
        }

    }
}

#endif//_INICE_NET_DEFINE
