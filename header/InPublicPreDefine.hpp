
#ifndef __PEYONE_NET_INPUBLICPREDEFINE_
#define __PEYONE_NET_INPUBLICPREDEFINE_


#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include <new>
#include <map>
#include <list>
#include <queue>
#include <string>
#include <utility>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

#ifdef WIN32
	#pragma warning( push )
	#pragma warning( disable : 4819 )
	#pragma warning( disable : 4244 )
	#include <boost/format.hpp>
	#pragma warning( pop )
	#include <sdkddkver.h>
#else
	#include <boost/format.hpp>
#endif


#include <boost/version.hpp>
#if BOOST_VERSION>107500 
	#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
		#define BOOST_BIND_GLOBAL_PLACEHOLDERS
	#endif
#endif

#include <typeinfo>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/timer/timer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/condition.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/system/system_error.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::posix_time;
using namespace boost::gregorian;

typedef boost::recursive_mutex::scoped_lock Boost_Scoped_Lock;
typedef boost::system::error_code			ZBoostErrCode;
typedef io_context::strand					AsioStrand;

typedef boost::function<void ()> NetHand_void_xcall;

namespace peony
{
	namespace net
	{
		const static unsigned cs_Define_256     = 256;
		const static unsigned cs_Define_128     = 128;
		const static unsigned cs_Define_64      = 64;
		const static unsigned cs_Define_32      = 32;
		const static unsigned cs_uIpLen         = 16;
		const static unsigned cs_sys_counter_id = 0;

		#define PN_CU_PATH "Peony::"

		typedef std::list<unsigned> LIST_UInt;
		typedef vector< string >   NVEC_STR;
		typedef map<string,string> NMAP_STR;

		typedef boost::function<void(class INiceNetServer*)> server_deallocator_type;
		typedef boost::function<void(unsigned,const void*,unsigned)>runmsg_handler_type;

		#define DELETE_OBJ(PA) if(PA) { delete (PA);(PA)=0;}

		class CInPubFun
		{
		public:
			static unsigned GetNowTime();
			static unsigned GetServerRelative();
			static unsigned GetNowTimeSs();
			static unsigned ymd_hms_str_to_curtime( string strv );
			static unsigned boostptime_to_usecond( boost::posix_time::ptime &myptime );
			static unsigned GetThreadId(boost::thread *pThread=0);
			static string   GetThreadIdStr();

			static void		SpliteStrToA( NVEC_STR &vStr, const char* pSource, string delim );
			static void		SpliteStrToB( NVEC_STR &vStr, string &sSource, string delim );
			static void		SpliteStrToMDes( NVEC_STR &vStr, string &sSource, string delimS ); //多个分隔符
			static bool		strcpy(char *pDes,unsigned umax,const char *pSour,unsigned icalpos/*=100*/);

            static string   LogExceptionMsgBuf(void *pBuffer, unsigned iCallPos);
		};

		class CLimit
		{
		public:
			static bool is_expired();
			static bool is_runMaxDuration();
		};

		class CZNetTimer
		{
		public:
			CZNetTimer();
			~CZNetTimer(){}

			float elapsed();
			void  restart();
	
		private:
			unsigned  m_LastTime;
		};

		class CPNetMark
		{//EmSocketMarketBase
		public:
			CPNetMark(){ m_XOMark=0; }
			virtual ~CPNetMark(){}

		public:
			virtual void	SetXMark( unsigned xmark ){ m_XOMark |= xmark; }
			virtual void	SetXMark( unsigned xmark,unsigned del_xmark )
			{
				m_XOMark &= (~del_xmark);
				m_XOMark |= xmark;
			}
			virtual void	ClearXMark(unsigned xmark){ m_XOMark &= (~xmark); }
			bool			IsXMark( unsigned xmark ){ return (m_XOMark&xmark)>0; }
			unsigned        GetXMarkV(){ return m_XOMark; }
			void            ClearAllXMark(){ m_XOMark=0; }
		protected:
			unsigned        m_XOMark;
		};

		struct TMainAppCallPos
		{
			unsigned    uLastCheckTm;   //上次检查的时间
			unsigned    uBeginCallTm;	//begin call time
			unsigned    uRunTmMaxLen;   //最大的允许运行时间，超过这个时间就报警
			char        strPos[128];
			TMainAppCallPos(){
				uLastCheckTm=0; uBeginCallTm=0; memset(strPos,0,sizeof(strPos)); uRunTmMaxLen=0;
			}
		};

		struct ZThreadItem
		{
			boost::thread *pThread;
			unsigned       uThreadID;
			ZThreadItem( boost::thread *pTh )
			{
				pThread   = pTh;
				uThreadID = 0;
				if( pTh ){
					uThreadID = CInPubFun::GetThreadId( pTh );
				}
			}
		};
		typedef std::list< ZThreadItem > ThreadPool;

		enum EmConnectInfo
		{
			CI_COMMON_ERROR=0,
			CI_ADDR_RESOVLE_FAILED,
			CI_ADDR_RESOVLE_SUCCESSED,
			CI_CONNECTION_FAILED,       //3
			CI_Connect_Success,         //4
			Both_SelfCloseConnect,      //5 我自己主动关闭发生的事件

			XS_Connect_Success = 10,
			XS_MeIsOpcTool,
		};

        extern unsigned g_MsgHeadLen;
	}
}

#endif
