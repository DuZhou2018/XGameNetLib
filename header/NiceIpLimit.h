#ifndef NiceIpLimit_h__
#define NiceIpLimit_h__
#include "../include/INiceNetCounter.h"
#include "./InPublicPreDefine.hpp"

namespace peony
{
    namespace net
    {
		typedef map<string,int>MapIpCount;
        class CIpLimitMg
        {
        public:
			CIpLimitMg();
            ~CIpLimitMg();
		public:
			static CIpLimitMg &Instance();

		public:
			void    SetIpMaxCt(int iMaxCt){ m_MaxCt=iMaxCt;}
			bool    IsAllocCon(string strIp);
			void	AddOneIp( string strIp  );
			void    DelOneIp( string strIp  );

		private:
			int						m_MaxCt;
			MapIpCount				m_MapIpCt;
			boost::recursive_mutex  m_mutex;
		};
    }
}
#endif
