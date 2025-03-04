#ifndef HFormData_h__
#define HFormData_h__

#include "../header/InPublicPreDefine.hpp"

namespace peony
{
	namespace net
	{
		class CHFormData
		{
			struct TBoundItem
			{
				char	chName[64];
				char    chFileName[64];
			};
		public:
			CHFormData( const char *pData, unsigned uDataL );
		public:
			int			Dowith( NMAP_STR &mapParam );

		private:
			int			GetBoundItem( TBoundItem &item,string &strSour );
			string      GetFieldStr_InOneLine( string &sSour,string strAMark,string strBMark );
		private:
			const char *m_pData;
			unsigned    m_iDataLen;
		};
	}
}

#endif
