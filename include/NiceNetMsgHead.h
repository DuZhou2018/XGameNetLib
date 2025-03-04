/********************************************************************
	created:	11:11:2013   14:41
	author:		ZhangHongtao  email: 1265008170@qq.com
	
	purpose:	通用的网络协议头(xuehai)

	struct TNiceNetMsgHead
	{
		unsigned	    Len;        //长度4 byte, 消息包长度, 包括包头的长度
		unsigned	    OPCode;     //长度4 byte, 消息编号
		unsigned	    CellID;     //长度4 byte, 逻辑链接成功后对应的CellID
		unsigned short  CheckCode;  //长度2 byte, 校验码,默认 21301
		unsigned short  BackID;     //长度2 byte, 备用,默认为0
		unsigned        ZIndex;     //长度4 byte, 消息索引编号,默认为0
	};

*********************************************************************/

#ifndef NiceNetMsgHead_h__
#define NiceNetMsgHead_h__
#include <memory.h>

namespace peony
{
    namespace net
    {
        //内部协议
        namespace INPROTOCOL
        {
			const unsigned short sc_THttpCheckCodeIn = 21301;

			#pragma pack(push)
			#pragma pack(push,1)

			//BackID 数据类型
			#define DK_MAPPED		0x01			//映射类型
			#define DK_WANGHUMSG    0x02			//网狐消息包结构
			#define DK_COMPRESS		0x04			//ZIP压缩类型

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
                unsigned	    Len;        //整个消息包长度
                unsigned	    OPCode;     //操作类别  
                unsigned	    UserID;    //
                unsigned short  CheckCode;  //校验吗21301;
                unsigned short  BackID;     //备用;
                unsigned        ZIndex;     //消息索引好
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
