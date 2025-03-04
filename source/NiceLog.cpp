#include "../header/NiceLog.h"
#include "../header/InPublicPreDefine.hpp"
#include "../header/ImpNetLog.h"
#include "../header/TCPPakDef.hpp"

namespace peony
{
    namespace net
    {
		INiceLog* INiceLog::CreateLog(  const char *fileName,
										const char *dirpath,
										unsigned uFileMaxSize /*=1024*1024*10*/,
										unsigned buffercount  /*=4*/)
        {
			CNiceLog *plog = new CNiceLog();
			if( plog && plog->Init( fileName,dirpath,uFileMaxSize,buffercount ) )
			{
				return plog;
			}
			delete plog;
            return 0;
        }

		void INiceLog::DeleteLog( INiceLog*& pLog )
		{
			if( 0==pLog )
				return;
			delete pLog;
			pLog = 0;
		}

        CNiceLog::CNiceLog(void)
        {
			m_pBase		   = 0;
			m_uLogGradeCon = LOG_Default;
			m_isnetlog     = false;
        }

        CNiceLog::~CNiceLog(void)
        {
			CloseLog();
        }
		void CNiceLog::CloseLog()
		{
			if( m_pBase )
			{
				m_pBase->stop();
				boost::this_thread::sleep( boost::posix_time::milliseconds(10) );
			}

			if( m_pBase )
			{
				delete m_pBase;
				m_pBase = 0;
			}
		}

		bool CNiceLog::Init(    string       nameLog,
								string       strdir,
								unsigned uFileMaxSize,
								unsigned buffercount)
        {
			if( 0==m_pBase )
			{
				m_pBase = new ImpLogToFile( strdir,nameLog,uFileMaxSize,buffercount );
				return true;
			}
			return false;
        }

        void CNiceLog::SetLogPutGrade( unsigned iSet )
        {
            Boost_Scoped_Lock lock(m_log_mutex);
            m_uLogGradeCon = iSet;
        }
        void CNiceLog::ClearLogGrade(unsigned uClearGrade)
        {
            Boost_Scoped_Lock lock(m_log_mutex);
            m_uLogGradeCon = m_uLogGradeCon&(~uClearGrade);
        }
        static string GetLogPreInfo(unsigned uGrade)
        {
            char chTempBuf[128]={0};
            memset(chTempBuf,0,sizeof(chTempBuf) );
            int index = LOG_ALL_LOG & uGrade;
            switch (index )
            {
            case Log_Error:
                {
                    sprintf(chTempBuf,"[%s]"," Err ");
                    break;
                }
            case Log_DebugNet:
                {
                    sprintf(chTempBuf,"[%s]"," DNet ");
                    break;
                }
            case Log_DebugCmd:
                {
                    sprintf(chTempBuf,"[%s]"," Cmd");
                    break;
                }
            case Log_DebugOther:
                {
                    sprintf(chTempBuf,"[%s]"," Oth ");
                    break;
                }
            case Log_DebugDeadlock:
                {
                    sprintf(chTempBuf,"[%s]","DLock");
                    break;
                }
            case Log_Warning:
                {
                    sprintf(chTempBuf,"[%s]"," War ");
                    break;
                }
            case Log_Normal:
                {
                    sprintf(chTempBuf,"[%s]"," BIn ");
                    break;
                }
			case Log_SysNormalInfo:
				{
					sprintf(chTempBuf,"[%s]"," Sys ");
					break;
				}
            default:
                sprintf(chTempBuf,"[%s]","Uknow");
                break;
            }

            string strRet = chTempBuf;

			/*
			{
			__time64_t time = _time64(NULL);
			struct tm  tmRet;
			struct tm* ptmTemp = &tmRet;
			_localtime64_s(&tmRet,&time);

			memset(chTempBuf,0,sizeof(chTempBuf) );
			strftime(chTempBuf, 256, " [%Y-%m-%d %H:%M:%S]: ", ptmTemp);
			strRet+=chTempBuf;
			}
			*/
			{
				//boost::date_time::date   today( boost::date_time::day_clock::local_day() );
				boost::posix_time::ptime curtime(boost::posix_time::second_clock::local_time());
				struct tm  tmRet   = boost::posix_time::to_tm( curtime );
				struct tm* ptmTemp = &tmRet;				
				memset(chTempBuf,0,sizeof(chTempBuf) );
				strftime(chTempBuf, 256, " [%Y-%m-%d %H:%M:%S]: ", ptmTemp);
				strRet+=chTempBuf;
			}

            return strRet;
        }
        bool CNiceLog::IsMayLog(unsigned uGrade )
        {
            bool bRet = !!(m_uLogGradeCon & uGrade );
            return bRet;
        }

     
        static boost::recursive_mutex	g_ScreenMutex;
        void  CNiceLog::PutLog(unsigned uGrade,stringstream &strIn)
        {
			Boost_Scoped_Lock MsgFun_lock(m_log_mutex);
			try
			{
				if( IsMayLog(uGrade) )
				{
					string strkind = GetLogPreInfo( uGrade );
					m_pBase->write_log( strkind.c_str(),     (unsigned)strkind.size() );
					m_pBase->write_log( strIn.str().c_str(), (unsigned)strIn.str().size() );

					if( PNGB::m_interfun_netlog )
					{
						PNGB::m_interfun_netlog( uGrade,strkind,strIn,m_isnetlog );
					}

					if( (m_uLogGradeCon & Log_PutOutScreen) || (uGrade & Log_Error) )
					{
						Boost_Scoped_Lock MsgFun_lock(g_ScreenMutex);
						ostringstream strlog;
						strlog<<strkind.c_str()<<strIn.str().c_str();
						printf( "%s",strlog.str().c_str() );
					}
				}
			}
			catch (...)
			{
				//std::cout<<"exception.......CClientLogInfo::PutLog()";
			}
        }
		void CNiceLog::RealtimeSaveLog()
		{
			Boost_Scoped_Lock MsgFun_lock(m_log_mutex);
			if( m_pBase )
				m_pBase->RealtimeSaveLog();
		}

		std::string CNiceLog::GetLogPreHead( unsigned uGrade )
		{
			return GetLogPreInfo( uGrade );
		}

		void WriteNetBufferMsgListInfo(unsigned ConID,vector<tcp_pak_header> &vecmsgheads )
		{
			ostringstream strmsgid;
			vector<tcp_pak_header>::const_iterator iter = vecmsgheads.begin();
			for(int icount=0;iter!=vecmsgheads.end(); ++iter )
			{
				++icount;
				strmsgid<<"["<<iter->GetMsgID()<<","<<iter->GetLen()<<"]";
				if( 0==icount%10  )
				{
					NETLOG_ERROR("[缓冲区信息]ConID="<<ConID<<strmsgid.str() );
					strmsgid.str("");
				}
			}

			NETLOG_ERROR("..[缓冲区信息]ConID="<<ConID<<strmsgid.str() );
		}
    }
}