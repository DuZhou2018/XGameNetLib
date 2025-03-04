#include "../header/NiceIpLimit.h"
#include "../include/INiceLog.h"
#include "../header/CGlobal.hpp"

namespace peony
{
    namespace net
    {

		CIpLimitMg::CIpLimitMg()
		{
			m_MaxCt = 100000;
		}

		CIpLimitMg::~CIpLimitMg()
		{

		}

		CIpLimitMg & CIpLimitMg::Instance()
		{
			static CIpLimitMg scIpLimitMg;
			return scIpLimitMg;
		}

		bool CIpLimitMg::IsAllocCon(string strIp)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			MapIpCount::iterator it = m_MapIpCt.find(strIp);
			if(it==m_MapIpCt.end())
				return true;
			if(it->second <= m_MaxCt)
				return true;
			NETLOG_ERROR("[同Ip限制已经最大] m_MaxCt=" << m_MaxCt<<"; strIp="<< strIp << FUN_LINE);
			return false;
		}

		void CIpLimitMg::AddOneIp(string strIp)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			MapIpCount::iterator it = m_MapIpCt.find( strIp );
			if(it==m_MapIpCt.end())
			{
				m_MapIpCt[strIp] = 1;
				return;
			}
			it->second = it->second+1;
		}

		void CIpLimitMg::DelOneIp(string strIp)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			MapIpCount::iterator it = m_MapIpCt.find(strIp);
			if(it==m_MapIpCt.end())
				return;
			if(it->second<=1)
			{
				m_MapIpCt.erase( strIp );
				return;
			}
			it->second = it->second-1;
		}

	}
}
