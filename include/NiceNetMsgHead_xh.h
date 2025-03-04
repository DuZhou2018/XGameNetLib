/********************************************************************
	created:	11:11:2013   14:41
	author:		ZhangHongtao  email: 1265008170@qq.com
	
	purpose:	ͨ�õ�����Э��ͷ(xuehai)
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
			//const unsigned short sc_THttpCheckCodeIn = 113;

			#pragma pack(push)
			#pragma pack(push,1)

			//TCP_Command::DataType ��������
			#define DK_MAPPED		0x01			//ӳ������
			#define DK_COMPRESS		0x04			//ZIPѹ������

			struct TNiceProHeadBase
			{
			public:
				TNiceProHeadBase(){ SetMsgID(0);SetLen(0);SetUserID(0);SetCheckCode(); SetBackID(0); }
				TNiceProHeadBase(unsigned MsgID){ SetMsgID(MsgID); SetLen(0);SetUserID(0);SetCheckCode();SetBackID(0); }
				TNiceProHeadBase(unsigned opcode, unsigned len){ SetMsgID(opcode); this->Len=len;SetUserID(0);SetCheckCode();SetBackID(0); }


				inline   unsigned		GetMsgID()const{ return OPCode;}
				inline   void			SetMsgID( unsigned v ){ OPCode = v;}
				inline   void			SetLen( unsigned uLen ){this->Len = (unsigned)uLen;}
				inline   unsigned		GetLen()const{ return this->Len; }
				inline   int			get_CheckCode(){return this->CheckCode; }
				inline   void			SetCheckCode(){ CheckCode = sc_THttpCheckCodeIn;}
				inline   void			SetBackID( short v ){ BackID=v; }
				inline   unsigned		GetBackID()const{ return BackID; }
				inline   void			SetUserID( unsigned v ){ user_id=v; }
				inline   unsigned		GetUserID()const{ return user_id; }

				//������ӵ�ֻ��Ϊ�˱���ͨ��
				inline   unsigned short GetMainMsgID()const{ return (OPCode>>16); }
				inline   unsigned short GetSubMsgID()const{  return OPCode-((OPCode>>16)<<16); }
				inline   unsigned		GetMsgIndex()const{  return 0;}
				inline   void			SetMsgIndex( unsigned v){}
				inline   long long		GetUserTempID()const{ return 0; }
				inline   void			SetUserTempID( long long v ){ }
				inline   unsigned short GetMsgDataType()const{ return 0; }


			protected:
			public:
				unsigned	OPCode;     //�������  
				unsigned	Len;        //������Ϣ������
				unsigned	user_id;    //
				unsigned short  CheckCode;  //У����21301;
				unsigned short  BackID;     //����;
			};

			//TCP_Head
			/***************************************************************************************
			struct TNiceProHeadBase : public TCP_Command
			{
			TNiceProHeadBase(){ SetMsgID(0);SetCheckCode(); }
			TNiceProHeadBase(unsigned MsgID){ SetMsgID(MsgID); SetCheckCode();}
			TNiceProHeadBase(unsigned opcode, unsigned len){ SetMsgID(opcode); this->Len=len;SetCheckCode(); }

			unsigned GetMsgID()const
			{
			unsigned  uMsgID = wMainCmdID<<16;
			uMsgID          += wSubCmdID;
			return uMsgID;
			}
			void SetMsgID( unsigned v )
			{
			wMainCmdID = v>>16;
			wSubCmdID  = v-(wMainCmdID<<16);
			}
			void SetMsgID( short a,short b )
			{
			wMainCmdID = a;
			wSubCmdID  = b;
			}
			void SetLen( unsigned uLen )
			{
			this->Len = (short)uLen;
			}
			int  get_CheckCode(){return this->cbCheckCode; }
			void SetCheckCode(){ cbCheckCode = sc_THttpCheckCodeIn;}
			};
			****************************************************************************************/

		   #pragma pack(pop)//INPROTOCOL
        }

    }
}

#endif//_INICE_NET_DEFINE
