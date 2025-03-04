#ifndef _INICE_NET_COUNTER_IMP
#define _INICE_NET_COUNTER_IMP
#include "../include/INiceNetCounter.h"
#include "./InPublicPreDefine.hpp"

namespace peony
{
    namespace net
    {
        class CNiceNetCounter : public INiceNetCounter
        {
            static const unsigned sc_StateRegister = 1;
            static const unsigned sc_StateInstall  = 1<<1;
        public:
            CNiceNetCounter(string Path,string Name,string Desc,
                            bool IsAutoReg,unsigned uServerID);
            virtual ~CNiceNetCounter();

        private:
            friend class CCountersMg;
            char            strPath[cs_Define_128];
            char            strName[cs_Define_32];
			char            strDesc[cs_Define_64];
            unsigned        uServer_instanceid;
            unsigned        uCounterID;
            unsigned        uMarkState;
			/********************************************************************
				created:	2009/11/10	10:11:2009   16:01
				author:		zhanghongtao	
				purpose:	发现一个问题，才添加了这个变量，因为有可能用户是中途订阅的
				计数器，造成数值为0的现象。
			*********************************************************************/
            unsigned    uBaseCounterValue;
        public:
            virtual bool IsRegrister()        { return !!(uMarkState&sc_StateRegister); }
            virtual bool Register();
            virtual void UnRegister();
            virtual void Update(unsigned uValue,EmCounterUpdateKind opKind);
 
			unsigned * GetBaseValue(){ return &uBaseCounterValue;           }
            void SetStateRegrister()  { uMarkState |=sc_StateRegister;          }
            void ClearStateRegrister(){ uMarkState &= (~sc_StateRegister);      }
			void SetStateInsatall()   { uMarkState |=sc_StateInstall;           }
            void ClearStateInstall()  { uMarkState &= (~sc_StateInstall);       }
            unsigned GetID()          { return uCounterID;                      }

        };
    }
}
#endif
