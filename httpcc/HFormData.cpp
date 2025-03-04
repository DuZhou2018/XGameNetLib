#include "./HFormData.hpp"

peony::net::CHFormData::CHFormData( const char *pData, unsigned uDataL )
{
	m_pData    = pData;
	m_iDataLen = uDataL;
}

int peony::net::CHFormData::Dowith( NMAP_STR &mapParam )
{
	//不处理文件
	NVEC_STR vAllLine;
	CInPubFun::SpliteStrToA(vAllLine,m_pData,"\r\n");
	if( vAllLine.size()<1 )
		return 2310101;

	string strBound=vAllLine[0];
	for( int t=0;t+2<(int)vAllLine.size(); t+=3 )
	{
		string strOne = vAllLine[t+0];
		if( string::npos == strOne.find(strBound) )
			continue;
		//开始一个节点
		string sName	= GetFieldStr_InOneLine(vAllLine[t+1],"name=\"","\"");
		string sData	= vAllLine[t+2];
		mapParam[sName] = sData;
	}
	return 0;
}

string peony::net::CHFormData::GetFieldStr_InOneLine( string &sSour,string strAMark,string strBMark )
{
	string::size_type sPosA = sSour.find(strAMark);
	if( string::npos==sPosA)
		return "";
	sPosA += strAMark.size();
	string::size_type sPosB = sSour.rfind(strBMark);
	if( string::npos==sPosB)
		return "";
	if( sPosA>=sPosB)
		return "";
	string strRet(sSour,sPosA,sPosB-sPosA );
	return strRet;
}
