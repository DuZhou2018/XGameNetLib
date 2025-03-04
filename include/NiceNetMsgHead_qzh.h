#ifndef NiceNetMsgHead_h__
#define NiceNetMsgHead_h__
#include <memory.h>

#define PEONYNET_PROTOCOL_QZH 1

namespace peony
{
    namespace net
    {
        //内部协议
        namespace INPROTOCOL
        {
			//const unsigned short sc_THttpCheckCodeIn = 21301;
			const unsigned sc_THttpCheckCodeIn = 69696969;

			#pragma pack(push)
			#pragma pack(push,1)

			//TCP_Command::DataType 数据类型
			#define DK_MAPPED		0x01			//映射类型
			#define DK_COMPRESS		0x04			//ZIP压缩类型

			//包头长度 40
			struct TCP_Command
			{
				unsigned        Len;				//数据包长度,包头+包体
				unsigned short  MCmdID;			    //消息ID
				unsigned short  SCmdID;             //消息ID
				unsigned        MsgIndex;			//消息编号
                unsigned        DataType;           //数据包的加密，压缩类型

				unsigned        user_id;	    	//用户ID， 备用，默认为0
				long long       UserTempID;			//用户的临时ID,千纸鹤用来保存一个连接的临时ID
				unsigned        BackID;			    //备用,
				unsigned        UnusedB;			//备用,现在是用来作为端口适用的
				int             CheckCode;			//效验码 69696969,或者别的固定值

				TCP_Command()
				{
					Len			= 0;
					MsgIndex	= 0;
					DataType	= 0;
					user_id		= 0;
					UserTempID	= 0;
					BackID      = 0;
					UnusedB     = 0;
					CheckCode   = 0;		
				}
			};

			//TCP_Head
			struct TNiceProHeadBase : public TCP_Command
			{
				TNiceProHeadBase(){ SetMsgID(0);SetCheckCode(); }
				TNiceProHeadBase(unsigned MsgID){ SetMsgID(MsgID); SetCheckCode();}
				TNiceProHeadBase(unsigned opcode, unsigned len){ SetMsgID(opcode); this->Len=len;SetCheckCode(); }

				unsigned GetMsgID()const
				{
					unsigned  uMsgID = MCmdID<<16;
					uMsgID          += SCmdID;
					return uMsgID;
				}
				void SetMsgID( unsigned v )
				{
					MCmdID = v>>16;
					SCmdID = v-(MCmdID<<16);
				}
				void SetMsgID( short a,short b )
				{
					MCmdID = a;
					SCmdID = b;
				}
				void SetLen( unsigned uLen )
				{
					this->Len = (short)uLen;
				}
				unsigned		GetLen()const{ return this->Len; }
				int				get_CheckCode(){return this->CheckCode; }
				void			SetCheckCode(){ CheckCode = sc_THttpCheckCodeIn;}

				void			SetBackID( short v ){ BackID=v; }
				long long		GetBackID()const{ return BackID; }
				void			SetUserID( unsigned v ){ user_id=v; }
				long long		GetUserID()const{ return user_id; }

				unsigned short	GetMainMsgID()const{ return MCmdID; }
				unsigned short	GetSubMsgID()const{  return SCmdID; }
				unsigned		GetMsgIndex()const{  return MsgIndex;}
				void			SetMsgIndex( unsigned v){ MsgIndex=v; }
				long long		GetUserTempID()const{ return UserTempID; }
				void			SetUserTempID( long long v ){ UserTempID=v; }
				void        	SetMsgDataType( unsigned v){ DataType=v; }
				unsigned short	GetMsgDataType()const{ return DataType; }

			};

		   #pragma pack(pop)//INPROTOCOL
        }

    }
}

#endif//_INICE_NET_DEFINE
