#include "./HAnGetParam.hpp"
#include "./UrlEncode.hpp"

peony::net::CHAnGetParam::CHAnGetParam( const char *pData, unsigned uDataL )
{
	m_pData    = pData;
	m_iDataLen = uDataL;
}

int peony::net::CHAnGetParam::Dowith( NMAP_STR &mapParam )
{
	//不处理文件
	NVEC_STR vAllLine;
	CInPubFun::SpliteStrToA(vAllLine,m_pData,"&");
	if( vAllLine.size()<1 )
		return 2310101;

	for( int t=0;t<(int)vAllLine.size(); t++ )
	{
		string  strOne = vAllLine[t];
		string::size_type saPos = strOne.find("=");
		if( string::npos == saPos )
			continue;
		string sName	= string(strOne,0,saPos);
		string sData	= string(strOne,saPos+1,strOne.size()-saPos-1);
		sData           = CUrlEncode::UrlDecode( sData );
		mapParam[sName] = sData;
	}
	return 0;
}
