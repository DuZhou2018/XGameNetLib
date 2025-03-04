#ifndef _INICE_LOG_IMP
#define _INICE_LOG_IMP

#include "../include/INiceLog.h"
#include "./CGlobal.hpp"

namespace peony
{
    namespace net
    {
        class CNiceLog : public INiceLog
        {
        public:
            CNiceLog(void);
            ~CNiceLog(void);

        public:
			bool  Init(     string       nameLog,
							string       strdir,
							unsigned uFileMaxSize,
							unsigned buffercount);

            virtual void  SetLogPutGrade( unsigned iSet );
			virtual unsigned  GetLogGrade(){ return m_uLogGradeCon;}                      
            virtual void  ClearLogGrade(unsigned uClearGrade);
            virtual bool  IsMayLog(unsigned uGrade );
            virtual void  PutLog(unsigned uGrade,stringstream &strIn);
			virtual void  RealtimeSaveLog();
			virtual void  SetNetLog(){ m_isnetlog = true; }

			string		  GetLogPreHead( unsigned uGrade );
		private:
			void		  CloseLog();
        private:
            unsigned			      m_uLogGradeCon;
			class ImpLogToFile		* m_pBase;
            boost::recursive_mutex	  m_log_mutex;
			bool                      m_isnetlog;
        };

        class CAutoLogDeadlock
        {
        public:
            CAutoLogDeadlock(string strFunName)
            {
#ifdef _DEBUG
				m_strFunName.clear();
				if( true )
				{
					m_strFunName=strFunName;
					m_strFunName+="();";
				}
				m_strFunName += strFunName;
				NETLOG_DEADLOCK(m_strFunName<<"(); begin...");
#endif
            }

            CAutoLogDeadlock(string strInfo,unsigned uParameOne,string strFunName)
            {
#ifdef _DEBUG
                stringstream strDb;
                m_strFunName.clear();
                if( true )
                {
                    strDb<<strFunName;
                    strDb<<"();";
                }
                strDb<<strInfo<<uParameOne;
                m_strFunName = strDb.str();
                NETLOG_DEADLOCK(m_strFunName<<"(); begin...");
#endif
            }

            ~CAutoLogDeadlock(void)
            {
#ifdef _DEBUG
                NETLOG_DEADLOCK(m_strFunName<<"(); end...");
                m_strFunName.clear();
#endif
            }
#ifdef _DEBUG
        private:
            string m_strFunName;
#endif
        };
    }
}
#endif
