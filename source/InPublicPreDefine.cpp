/****************************************************************************************************
created:	2010-8-24   16:55
filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\source\InPublicPreDefine.cpp
author:		Zhanghongtao

purpose:	 
*****************************************************************************************************/
#include "../header/InPublicPreDefine.hpp"
#include "../header/CGlobal.hpp"
#include "../include/INiceLog.h"


namespace peony
{
	namespace net
	{
		unsigned g_RunMsgEvent = 0;
        unsigned g_MsgHeadLen  = 20;

		unsigned CInPubFun::GetNowTime()
		{
			static boost::posix_time::ptime scBegin( boost::gregorian::date(1970, boost::gregorian::Jan, 1) );
			//boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::local_time()-scBegin;
			boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time()-scBegin;
			unsigned uSecond = (unsigned)time_from_epoch.total_seconds();
			return uSecond;
		}
		unsigned CInPubFun::GetServerRelative()
		{
			#ifdef WIN32
				struct _timeb  timebuffer;
				_ftime(&timebuffer);
			#else
				struct timeb  timebuffer;
				ftime(&timebuffer);
			#endif
			static boost::int64_t scbegin_scond  = (boost::int64_t)timebuffer.time; //开始的秒
			int iMillTM      = timebuffer.millitm;
			boost::int64_t relativetime = (timebuffer.time-scbegin_scond)*100+iMillTM/10;

			unsigned server_relativetime = (unsigned)relativetime;
			return server_relativetime;
		}

		unsigned CInPubFun::GetNowTimeSs()
		{
			static boost::posix_time::ptime  time_t_epoch( boost::posix_time::time_from_string("1970-01-01 00:00:00") );
			boost::posix_time::ptime         temp( boost::posix_time::second_clock::local_time() );
			boost::posix_time::time_duration duration = temp-time_t_epoch;
			unsigned uRet = (unsigned)duration.total_microseconds()/10000;
			return uRet;
		}

		unsigned CInPubFun::ymd_hms_str_to_curtime( string strv )
		{
			//0000-0-00
			//2011-3-18 3:58:02
			if( strv.size()<9 )
			{
				NETLOG_ERROR("[时间转换] 数据格式错误: strv="<<strv<<FUN_FILE_LINE );
				return 0;
			}

			try
			{
				boost::posix_time::ptime ptmy( boost::posix_time::time_from_string(strv) );
				return boostptime_to_usecond( ptmy );
			}
			catch (...)
			{
				NETLOG_ERROR("[时间转换] 数据格式异常: strv="<<strv<<FUN_FILE_LINE );
				return 0;
			}

			return 0;
		}
		unsigned CInPubFun::boostptime_to_usecond( boost::posix_time::ptime &myptime )
		{
			static boost::posix_time::ptime time_t_epoch( boost::posix_time::time_from_string("1970-01-01 00:00:00") );
			boost::posix_time::time_duration td = myptime-time_t_epoch;
			return (unsigned)td.total_seconds();
		}

		unsigned GFunPNetGetThreadID( boost::thread::id myTID )
		{
			ostringstream ostr;
			ostr<<myTID;
			string strhhid = ostr.str();
			char szValue[64] = {0};
			if( strhhid.size()>strlen(szValue)-1)
				return 0;
			memcpy(szValue,strhhid.c_str(),strhhid.size());
			unsigned nValude = 0;
			sscanf(szValue,"%x",&nValude);
			return nValude;
		}
		unsigned CInPubFun::GetThreadId( boost::thread *pThread )
		{	
			boost::thread::id myTID;
			if( 0==pThread ){
				myTID =	boost::this_thread::get_id();
			}else{
				myTID =	pThread->get_id();
			}
			return GFunPNetGetThreadID( myTID );
		}

		string CInPubFun::GetThreadIdStr()
		{
			string strTempAa = boost::str( boost::format(" [ ThreadID=%d ] ")%GetThreadId() );
			return strTempAa;
		}

		void CInPubFun::SpliteStrToA( NVEC_STR &vStr, const char* pSource, string delim )
		{
			if( !pSource )
				return;
			string sSource = pSource;
			SpliteStrToB( vStr,sSource,delim );
		}

		void CInPubFun::SpliteStrToB( NVEC_STR &vStr, string &sSource, string delim )
		{
			if( sSource.empty() || delim.empty() )
				return;

			std::string::size_type deli_len = delim.size();
			std::string::size_type index = -1,npos = -1, last_search_position = 0;
			while( (index=sSource.find(delim,last_search_position))!=npos )
			{
				if(index!=last_search_position){
					vStr.push_back( sSource.substr(last_search_position, index-last_search_position) );
				}
				last_search_position = index + deli_len;
			}

			string last_one = sSource.substr(last_search_position);
			if( last_one.size()>0 )
			{
				vStr.push_back( last_one );
			}
		}

		void CInPubFun::SpliteStrToMDes( NVEC_STR &vStr, string &sSource, string delimS )
		{
			if( sSource.empty() || delimS.empty() )
				return;

			std::string::size_type index = -1,npos = -1, last_search_position = 0;
			while( (index=sSource.find_first_of(delimS,last_search_position))!=npos )
			{
				if(index!=last_search_position){
					vStr.push_back( sSource.substr(last_search_position, index-last_search_position) );
				}
				last_search_position = index + 1;
			}

			string last_one = sSource.substr(last_search_position);
			if( last_one.size()>0 )
			{
				vStr.push_back( last_one );
			}
		}

		bool CInPubFun::strcpy(char *pDes,unsigned umax,const char *pSour,unsigned icalpos/*=100*/)
		{
			if( (!pDes) || (!pSour) )
			{
				NETLOG_ERROR("有空指针存在!"<<FUN_FILE_LINE);
				return false;
			}

			if( pDes == pSour )
			{
				NETLOG_ERROR("请不要自己拷贝自己!"<<FUN_FILE_LINE);
				return false;
			}

			if( umax>=strlen(pSour)+1 )
			{
				memset(pDes,0,umax);
				strncpy(pDes,pSour,strlen(pSour));
				return true;
			}
			if( strlen(pSour)<512 )
			{
				NETLOG_ERROR("[目标字符串太长了!]<0> umax="<<umax<<",sourlen="<<strlen(pSour)<<";pSour="<<pSour<<"; icalpos="<<icalpos<<FUN_FILE_LINE);
			}else
			{ 
				NETLOG_ERROR("[目标字符串太长了!] umax="<<umax<<",sourlen="<<strlen(pSour)<<"; icalpos="<<icalpos<<FUN_FILE_LINE);
			}
			return false;
		}

        string CInPubFun::LogExceptionMsgBuf(void *pBuffer, unsigned iCallPos)
        {
            ostringstream strLog;
            INPROTOCOL::TInNiceNetHead *pHeadBase = (INPROTOCOL::TInNiceNetHead *)pBuffer;
            //NETLOG_FATAL("[消息处理异常.net.tcp],"<<LogSelf()<<" OPCode[ "<<pHeadBase->GetMsgID()<<" ]");
            strLog<<"[消息处理异常.net.tcp], OPCode=[ "<<pHeadBase->GetMsgID()<<" ]";
            return strLog.str();
            //return "DoMsgException390503209!!";
        }

        bool peony::net::CLimit::is_expired()
		{
			static unsigned sc_ExpiredTime = CInPubFun::ymd_hms_str_to_curtime("2026-10-1 01:01:01");
			PNGB::m_zhtc_slowertime = sc_ExpiredTime;
			if( PNGB::m_server_curTm>sc_ExpiredTime )
				return true;
			return false;
		}

		bool CLimit::is_runMaxDuration()
		{
			return false;

			//4个小时
			static unsigned scumaxrunduration = 3600*4*6*100;
			static unsigned ubegintime = CInPubFun::GetNowTime();
			if( CInPubFun::GetNowTime()-ubegintime>scumaxrunduration )
				return true;
			return false;
		}

		CZNetTimer::CZNetTimer()
		{
			restart();
		}

		float CZNetTimer::elapsed()
		{
			float fvv = (float)(PNGB::m_server_relativeTm-m_LastTime);
			return fvv/100.0f;
		}

		void CZNetTimer::restart()
		{
			m_LastTime = PNGB::m_server_relativeTm;
		}

	}
}