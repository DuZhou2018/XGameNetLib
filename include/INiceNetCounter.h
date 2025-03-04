/********************************************************************
	created:	2:7:2010   16:31
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\INiceNetCounter.h
	author:		Zhanghongtao
	
	purpose:	提供计数器服务，目前关闭，因为协议号的问题。
*********************************************************************/
#ifndef _INICE_NET_COUNTER
#define _INICE_NET_COUNTER
#include "./NiceNetDefine.h"

namespace peony
{
    namespace net
    {
        enum EmCounterUpdateKind
        {
            CuUpKind_Add,
            CuUpKind_Sub,
            CuUpKind_Replace,
            CuUpKind_Swap,
        };
        class INiceNetCounter
        {
        public:
            //need you to delete
            static INiceNetCounter *CreateCounter( const char * strPath,
                                                   const char * strName,
												   const char * strDesc,
                                                   bool   IsAutoReg = true,
                                                   class INiceNetServer *pServer=0);
// 		   static INiceNetCounter *CreateCounter( string strPath,
// 													   string strName,
// 													   string strDesc,
// 													   bool   IsAutoReg = true,
// 												   class INiceNetServer *pServer=0);
 		    static void            DeleteCounter( INiceNetCounter *pCounter );
        protected:
            INiceNetCounter(){}
            virtual ~INiceNetCounter(){}
        public:
            virtual bool IsRegrister()  =0;
            virtual bool Register()     =0;
            virtual void UnRegister()   =0;
            virtual void Update(unsigned uValue,EmCounterUpdateKind opKind)=0;

            void UpdateAdd(unsigned uValue){this->Update(uValue,CuUpKind_Add);}
            void UpdateSub(unsigned uValue){this->Update(uValue,CuUpKind_Sub);}
            void UpdateReplace(unsigned uValue){this->Update(uValue,CuUpKind_Replace);}
            void UpdateSwap(unsigned uValue){this->Update(uValue,CuUpKind_Swap);}
            
        };

#define DELETE_COUNTER( A )\
	do{\
		if( A ){INiceNetCounter::DeleteCounter( A );A=0;}\
	  }while(0)
    }
}
#endif
