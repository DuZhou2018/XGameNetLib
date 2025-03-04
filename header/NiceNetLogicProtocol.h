#ifndef NiceNetLogicProtocol_h__
#define NiceNetLogicProtocol_h__

#include "../include/NiceNetDefine.h"

namespace peony
{
    namespace net
    {
		//内部协议
		namespace INPROTOCOL
		{
#pragma pack(push)
#pragma pack(push,1)
			enum EmDefineConst
			{
				IP_LENGTH = 16,
				COUNTERLIST_MAXCOUNT = 10,
			};
			enum EnNiceNetInPro
			{//   3997697000 = 61000, 1000 
				NiceNetP_INVALID_MSGID            = 3997697000, //无校msgID =36865

				//计数器协议
				NiceNetP_Counter_CTOS_ReqList     = 3997697001,
				NiceNetP_Counter_CTOS_ReqBook     = NiceNetP_Counter_CTOS_ReqList+1,
				NiceNetP_Counter_STOC_RepBook     = NiceNetP_Counter_CTOS_ReqBook+1,
				NiceNetP_Counter_STOC_RepList     = NiceNetP_Counter_STOC_RepBook+1,
				NiceNetP_Counter_STOC_STATE       = NiceNetP_Counter_STOC_RepList+1,

				//网络log协议
				NiceNetP_NetLog_OpReader_Req      = 3997697021,    //
				NiceNetP_NetLog_OpReader_Rep      = 3997697022,    //
				NiceNetP_NetLog_SendLog           = 3997697023,    //发送log给tool

				//网络控制协议
				NiceNetP_ZhtControl_Begin         = 3997697040,
				NiceNetP_ZhtControl_LogicReq,
				NiceNetP_ZhtControl_LogicRep,
				NiceNetP_ZhtControl_CloseMeNetHeart,			   //61000,1043=3997697043关闭我自己的网络层心跳检查
			};

			//////////////////////////////////////////////////////////////////////////
			typedef struct _ClientReqCounterList : public TInNiceNetHead
			{
				_ClientReqCounterList():TInNiceNetHead(NiceNetP_Counter_CTOS_ReqList, sizeof(_ClientReqCounterList)){ }
			}TClientReqCounterList;

			typedef struct _ServerRepCounterList : public TInNiceNetHead
			{
				struct COUNTER_ITEM
				{
					unsigned short id;
					char name[32];
					char path[128];
					char desc[64];
				};

				_ServerRepCounterList():TInNiceNetHead(NiceNetP_Counter_STOC_RepList){}
				unsigned short   count;
				COUNTER_ITEM items[COUNTERLIST_MAXCOUNT];
			}TServerRepCounterList;
			typedef std::list<TServerRepCounterList> LIST_RepCounterList;

			typedef struct _ClientReqBook : public TInNiceNetHead
			{
				_ClientReqBook():TInNiceNetHead(NiceNetP_Counter_CTOS_ReqBook, sizeof(_ClientReqBook)){}
				unsigned short counter_id;
				int interval;
			}TClientReqBook;

			typedef struct _ServerRepBook : public TInNiceNetHead
			{
				_ServerRepBook():TInNiceNetHead(NiceNetP_Counter_STOC_RepBook, sizeof(_ServerRepBook) ){}
				unsigned short counter_id;
				int            is_success; //0:退订,小于0:错误,大于0:当前订阅周期
			}TServerRepBook;

			typedef struct _ServerNoticeState : public TInNiceNetHead
			{
				struct COUNTER_ITEM_STATE
				{
					unsigned short counter_id;
					int value;
				};
				_ServerNoticeState():TInNiceNetHead(NiceNetP_Counter_STOC_STATE){}

				short count;
				COUNTER_ITEM_STATE items[COUNTERLIST_MAXCOUNT];
			}TServerNoticeState;
			typedef std::list<TServerNoticeState> LIST_RepCounterState;

			/////Reader //////////////////////////////////////////////////////////////
			/////Reader //////////////////////////////////////////////////////////////
			enum emNiceNetPNetLogOpReaderReqType
			{
				emNiceNetPNetLogOpReaderType_Add = 1,
				emNiceNetPNetLogOpReaderType_Del,
				emNiceNetPNetLogOpReaderType_ModifyGrade,
			};
			typedef struct _NiceNetPNetLogOpReaderReq : public TInNiceNetHead
			{
				emNiceNetPNetLogOpReaderReqType reqtype;
				unsigned					appgrade;
				unsigned					netgrade;
				_NiceNetPNetLogOpReaderReq():TInNiceNetHead(NiceNetP_NetLog_OpReader_Req){}

			}TNiceNetPNetLogOpReaderReq;

			enum emNiceNetPNetLogOpReaderResult
			{
				emNiceNetPNetLogOpReaderResult_OK = 1,
				emNiceNetPNetLogOpReaderResult_Fail,
			};
			typedef struct _NiceNetPNetLogOpReaderRep : public TInNiceNetHead
			{
				emNiceNetPNetLogOpReaderReqType  reqtype;
				emNiceNetPNetLogOpReaderResult   result;
				_NiceNetPNetLogOpReaderRep():TInNiceNetHead(NiceNetP_NetLog_OpReader_Rep){}

			}TNiceNetPNetLogOpReaderRep;

			typedef struct _NiceNetPNetLogSendLog : public TInNiceNetHead
			{
				unsigned index;
				unsigned datalen;
				char		 buffer[1024];
				_NiceNetPNetLogSendLog():TInNiceNetHead(NiceNetP_NetLog_SendLog){}

			}TNiceNetPNetLogSendLog;

			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			struct NiceNetPZhtControlLogicReq : public TInNiceNetHead
			{
				static const unsigned sczhtcontrlmarked = 210379213;
				char          zhtcontroltype[64];
				unsigned  marked; //=210379213
				NiceNetPZhtControlLogicReq():TInNiceNetHead(NiceNetP_ZhtControl_LogicReq,sizeof(NiceNetPZhtControlLogicReq))
				{
					marked = 0;
				}
			};
			struct NiceNetPZhtControlLogicRep : public TInNiceNetHead
			{
				unsigned  result;
				unsigned  marked; //=210379213
				NiceNetPZhtControlLogicRep():TInNiceNetHead(NiceNetP_ZhtControl_LogicRep,sizeof(NiceNetPZhtControlLogicRep)){}
			};
			struct NiceNetPZhtControlCloseMeNetHeartReq : public TInNiceNetHead
			{//msgid = 3997697043
				bool IsOpen;
				NiceNetPZhtControlCloseMeNetHeartReq():TInNiceNetHead(NiceNetP_ZhtControl_CloseMeNetHeart,sizeof(NiceNetPZhtControlCloseMeNetHeartReq))
				{
					IsOpen = 0;
				}
			};
#pragma pack(pop)//INPROTOCOL
		}
    }
}

#endif//NiceNetLogicProtocol_h__
