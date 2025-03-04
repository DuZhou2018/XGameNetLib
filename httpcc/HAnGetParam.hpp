#ifndef HAnGetParam_h__
#define HAnGetParam_h__
#include "../header/InPublicPreDefine.hpp"

namespace peony
{
	namespace net
	{
		class CHAnGetParam
		{
		public:
			CHAnGetParam( const char *pData, unsigned uDataL );
		public:
			int			Dowith( NMAP_STR &mapParam );

		private:
			const char *m_pData;
			unsigned    m_iDataLen;

		};
	}
}

#endif
