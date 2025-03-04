/********************************************************************
	created:	2:7:2010   16:28
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\INiceLog.h
	author:		Zhanghongtao
	
	purpose:	提供安全可靠的log模块。支持多线程
*********************************************************************/
#ifndef _INICE_LOG
#define _INICE_LOG
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

namespace peony
{
    namespace net
    {
        //define log grad;
        const unsigned   Log_Error           = 1<<0;      //1 normal error，may run
		const unsigned   Log_Warning         = 1<<1;      //2
		const unsigned   Log_SysNormalInfo   = 1<<2;      //4
		const unsigned   Log_PutOutScreen    = 1<<3;      //8
		const unsigned   Log_Normal          = 1<<4;      //
        const unsigned   Log_DebugNet        = 1<<5;      //debug for net
        const unsigned   Log_DebugCmd        = 1<<6;      //debug for cmd
        const unsigned   Log_DebugOther      = 1<<7;      //debug for cmd
        const unsigned   Log_DebugDeadlock   = 1<<8;      // 1<<8=256

		const unsigned   LOG_ALL_DEBUG       = Log_DebugNet |Log_DebugCmd |Log_DebugOther|Log_Normal|Log_PutOutScreen;	//34
		const unsigned   LOG_ALL_ERROR       = Log_Error|Log_Warning|Log_PutOutScreen;						//3
        const unsigned   LOG_Default         = LOG_ALL_ERROR|Log_Warning|Log_SysNormalInfo|Log_PutOutScreen;						//
		const unsigned   LOG_ALL_LOG         = LOG_ALL_ERROR|LOG_ALL_DEBUG|LOG_Default|Log_Normal|Log_SysNormalInfo;						//
        
		class INiceLog
        {
        public:
			static INiceLog* CreateLog( const char *fileName,
										const char *dirpath,
										unsigned uFileMaxSize =1024*1024*10,
										unsigned buffercount  =4);
        protected:
            INiceLog(){}
        public:
            virtual ~INiceLog(){}
            virtual void   SetLogPutGrade( unsigned iSet )     = 0;
			virtual unsigned  GetLogGrade()                   = 0;
            virtual void   ClearLogGrade(unsigned uClearGrade) = 0;
            virtual bool   IsMayLog(unsigned uGrade )          = 0;
            virtual void   PutLog(unsigned uGrade,stringstream &strIn)=0;
			virtual void   RealtimeSaveLog()          = 0;
			virtual void   SetNetLog()                = 0;
			virtual string GetLogPreHead( unsigned uGrade )=0;
			static	void   DeleteLog(peony::net::INiceLog*&);

        };

#define PEYONE_REALTIMESAVE_LOG(pLog)\
	do{	if( pLog) pLog->RealtimeSaveLog();}while(0)
		 

#define PEYONE_LOG_MSG(pLog,MSG,GRADE)\
	do{\
		if(0==pLog){\
			stringstream strStream;\
			strStream<<MSG<<endl;\
			std::cout<<strStream.str().c_str();\
		}\
		if( pLog &&pLog->IsMayLog(GRADE))\
		{\
		    try{\
				stringstream strStream;\
				strStream<<MSG<<endl;\
				pLog->PutLog(GRADE,strStream);\
		    }catch(...){\
				std::cout<<"log exception...XXXXXX...."<<__FUNCTION__;\
			}\
		}\
	}while(0)


#define NETLOG_FATAL(MSG)       PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_Error)
#define NETLOG_ERROR(MSG)       PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_Error)
#define NETLOG_DBNET(MSG)       PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_DebugNet)
#define NETLOG_DBCMD(MSG)       PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_DebugCmd)
#define NETLOG_DBOTHER(MSG)     PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_DebugOther)
#define NETLOG_WARNING(MSG)     PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_Warning)
#define NETLOG_NORMAL(MSG)      PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_Normal)
#define NETLOG_DEADLOCK(MSG)    PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_DebugDeadlock)
#define NETLOG_SYSINFO(MSG)     PEYONE_LOG_MSG(PNGB::m_pLog,"[ net ] "<<MSG,Log_SysNormalInfo)
#define PYYLOG_ERROR(MSG)       PEYONE_LOG_MSG(PNGB::m_pLog,"[ py ] "<<MSG,Log_Error)
#define PYYLOG_SYSINFO(MSG)     PEYONE_LOG_MSG(PNGB::m_pLog,"[ py ] "<<MSG,Log_SysNormalInfo)
#define NETLOG_SAVELOG()        PEYONE_REALTIMESAVE_LOG(PNGB::m_pLog)
        

#define FUN_FILE_LINE " ;"<<__FUNCTION__<<"();  File:"<<__FILE__<<" Line:"<<__LINE__
#define FUN_LINE      " ;"<<__FUNCTION__<<", (Line:"<<__LINE__<<" ):"


//Here apply try kind
#define TRY_BEGIN_CPLUS_NET	try{

#define TRY_END_CPLUS_NET	   }\
		catch(...) \
		{ \
			NETLOG_FATAL("[try...],"<<FUN_FILE_LINE);\
		}

#define TRY_END_CPLUS_RETURN_NET(A) }\
		catch(...) \
		{ \
			NETLOG_FATAL("[try...],"<<FUN_FILE_LINE);\
			return (A);\
		}

    }
}

#endif
