#include "../header/NiceNetCounter.h"
#include "../header/INiceNetServer.h"
#include "../header/CGlobal.hpp"
#include "../header/CountersMg.hpp"
#include "../include/INiceNet.h"
#include "../header/NiceLog.h"
#include "../header/NiceNet.h"

namespace peony
{
    namespace net
    {
        CNiceNetCounter::CNiceNetCounter(string Path,string Name,string Desc,bool IsAutoReg,unsigned uServerID)
        {
			uBaseCounterValue = 0;
            this->uMarkState = 0;
            this->uCounterID = 0;
            memset((void*)this->strPath,0,sizeof(this->strPath));
            memset((void*)this->strName,0,sizeof(this->strName)) ;
			memset((void*)this->strDesc,0,sizeof(this->strDesc) );
            if(    Path.size()<sizeof(this->strPath)
				&& Name.size()<sizeof(this->strName)
				&& Desc.size()<sizeof(this->strDesc) )
            {
                strncpy(this->strPath,Path.c_str(),Path.size());
                strncpy(this->strName,Name.c_str(),Name.size());
				strncpy(this->strDesc,Desc.c_str(),Desc.size());
            }else
			{
				strncpy(this->strPath,Path.c_str(),sizeof(this->strPath)-1);
				strncpy(this->strName,Name.c_str(),sizeof(this->strName)-1);
				strncpy(this->strDesc,Desc.c_str(),sizeof(this->strDesc)-1);
				NETLOG_WARNING("新添加的计数器名称太长,被截断;"<<FUN_FILE_LINE);
			}

			this->uServer_instanceid = uServerID;
			if( IsAutoReg )
				this->Register();
        }

        CNiceNetCounter::~CNiceNetCounter()
        {
            //CAutoLogDeadlock AutoLgDL(__FUNCTION__); 
            this->UnRegister();
        }
        bool CNiceNetCounter::Register()
        {
            PNGB::m_pCountersMg->RegisterCounter( *this );
            return true;
        }
        void CNiceNetCounter::UnRegister()
        {
            if(this->IsRegrister())
            {
                CNiceNet *pNet = dynamic_cast<CNiceNet*>(&INiceNet::Instance());
                pNet->LogoutCounter( this );
            }
        }
        void CNiceNetCounter::Update(unsigned uValue,EmCounterUpdateKind opKind)
        {
			CCountersMg::CuMg_UpdateValue(uBaseCounterValue,uValue,opKind);
            //if( this->IsInstall() )
            //    PNGB::m_pCountersMg->UpdateCounterValue(*this,uValue,opKind);
        }

    }
}

