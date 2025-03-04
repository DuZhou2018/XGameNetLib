/**********************************************************************************************************************************************
created:	2018/01/17 10:03
filename:   NiceNetMsgHead.h
author:		ZhangHongtao  email: 1265008170@qq.com

purpose:	千纸鹤的第二版本
************************************************************************************************************************************************/

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
			const unsigned sc_THttpCheckCodeIn = 0;

#pragma pack(push)
#pragma pack(push,1)

			//TCP_Command::DataType 数据类型
#define DK_MAPPED		0x01			//映射类型
#define DK_COMPRESS		0x04			//ZIP压缩类型

			//包头长度
			struct TCP_Command
			{
				unsigned short					wEnPacketSize;						//包头+数据长度，网络字节顺序

				char							dwClientPort[32];					//唯一标识
				unsigned short					wMsgType;							//消息类型
				unsigned						dwConnectPort;						//连接端口
				long long						ulCmdNo;							//包头序号

				unsigned						dwGameID;							//玩家标识
				char							secondKey[4];						//二次验证

				char							cbDataKind;							//数据类型
				char							cbCheckCode;						//效验字段
				unsigned short  				wPacketSize;						//数据大小

				unsigned short					MCmdID;			    //消息ID
				unsigned short					SCmdID;             //消息ID

				TCP_Command()
				{
					wEnPacketSize = sizeof(TCP_Command) - 4;
					wMsgType = 0;
					dwConnectPort = 0;
					ulCmdNo = 0;
					dwGameID = 0;
					cbDataKind = 0;
					cbCheckCode = 0;
					wPacketSize = 0;
					memset(dwClientPort, 0, sizeof(dwClientPort));
					memset(secondKey, 0, sizeof(secondKey));
				}
			};

			//TCP_Head
			struct TNiceProHeadBase : public TCP_Command
			{
				TNiceProHeadBase() { SetMsgID(0);SetCheckCode(); }
				TNiceProHeadBase(unsigned MsgID) { SetMsgID(MsgID); SetCheckCode(); }
				TNiceProHeadBase(unsigned MsgID, unsigned len) { SetMsgID(MsgID); SetLen(len); SetCheckCode(); }

				unsigned GetMsgID()const
				{
					unsigned  uMsgID=MCmdID<<16;
					uMsgID+=SCmdID;
					return uMsgID;
				}
				void SetMsgID(unsigned v)
				{
					MCmdID=v>>16;
					SCmdID=v-(MCmdID<<16);
				}
				void SetMsgID(short a, short b)
				{
					MCmdID=a;
					SCmdID=b;
				}
				void SetLen(unsigned uLen)
				{
					this->wPacketSize=(short)uLen;
                    unsigned short usPackLen = this->wPacketSize-2;
					char *pAaa=(char*)&usPackLen;
					char *pBbb=(char*)&wEnPacketSize;
					*pBbb=*(pAaa+1);
					*(pBbb+1)=*pAaa;
				}
				unsigned		GetLen()const { return this->wPacketSize; }
				int				get_CheckCode() { return this->cbCheckCode; }
				void			SetCheckCode() { cbCheckCode=(char)sc_THttpCheckCodeIn; }

				void			SetBackID(short v) { wMsgType=v; }
				long long		GetBackID()const { return wMsgType; }
				void			SetUserID(unsigned v) { wMsgType=v; }
				long long		GetUserID()const { return wMsgType; }

				unsigned short	GetMainMsgID()const { return MCmdID; }
				unsigned short	GetSubMsgID()const { return SCmdID; }
				unsigned		GetMsgIndex()const { return ulCmdNo; }
				void			SetMsgIndex(unsigned v) { ulCmdNo=v; }
				long long		GetUserTempID()const { return dwGameID; }
				void			SetUserTempID(long long v) { dwGameID=v; }
				void        	SetMsgDataType(unsigned v) { cbDataKind=v; }
				unsigned short	GetMsgDataType()const { return cbDataKind; }

			};

#pragma pack(pop)//INPROTOCOL
		}

	}
}

#endif//_INICE_NET_DEFINE
