#include "../include/NiceNetDefine.h"
#include "../include/INiceNet.h"
#include "../header/NiceHttpReq.h"
#include "../header/NiceLog.h"
#include "../httpcc/UrlEncode.hpp"
#include "../httpcc/HFormData.hpp"
#include "../httpcc/HAnGetParam.hpp"


namespace peony
{
	namespace net
	{
		CNHttpReq::CNHttpReq( void )
		{
			m_Index = 1;
			m_ConID = 0;
			memset(&m_BaseInfo,0,sizeof(m_BaseInfo) );
		}

		CNHttpReq::~CNHttpReq( void )
		{

		}

		const char* CNHttpReq::GetPostData( unsigned &uDataLen )
		{
			uDataLen = 0;
			if( !this->IsPost() )
				return 0;

			if( m_BaseInfo.PostDataBeginPos+m_BaseInfo.Content_Length>m_BaseInfo.RecBuf_DataLen )
				return 0;

			uDataLen = m_BaseInfo.Content_Length;
			return m_pBufBeginPos+m_BaseInfo.PostDataBeginPos;
		}
		void CNHttpReq::SetRepContentType( string strCType )
		{
			CInPubFun::strcpy(m_BaseInfo.Rep_ContentType,sizeof(m_BaseInfo.Rep_ContentType),strCType.c_str(),656405);
		}

		string CNHttpReq::InFun_BuildHttpHead()
		{
			ostringstream strHead;
			strHead<<"HTTP/1.0 200 ZhtCppWebServiceRetOk\r\n";
			strHead<<"Server: ZhtWebService 1.0.0\r\n";
			if( strlen(m_BaseInfo.Rep_ContentType)>0 )
			{
				strHead<<"Content-Type: "<<m_BaseInfo.Rep_ContentType<<"\r\n";
			}else{
				strHead<<"Content-Type: text/html\r\n";
			}

			strHead<<"\r\n";
			return strHead.str();
		}

		int CNHttpReq::RepMsg( const char *pData,unsigned uDataLen )
		{
			static  char sc_http_repbuf[10240000]={0};
			memset(sc_http_repbuf,0,sizeof(sc_http_repbuf) );

			if( uDataLen+2048>sizeof(sc_http_repbuf) )
			{
				NETLOG_ERROR("LogicError http repmsg maxsize 10M ! "<<FUN_LINE );
				return 9302385;
			}
			memset( sc_http_repbuf,0, sizeof(sc_http_repbuf) );

			unsigned uCurPos = 0;
			tcp_pak_header *pMsg =(tcp_pak_header*)sc_http_repbuf;
			pMsg->SetMsgID(999999);
			pMsg->SetCheckCode();
			uCurPos += sizeof(tcp_pak_header);

			string strHttpHead = InFun_BuildHttpHead();
			CInPubFun::strcpy( sc_http_repbuf+uCurPos,sizeof(sc_http_repbuf)-uCurPos,strHttpHead.c_str(),4324301 );
			uCurPos += (unsigned)strHttpHead.size();

			memcpy( sc_http_repbuf+uCurPos,pData,uDataLen );
			uCurPos += uDataLen;
			pMsg->SetLen(uCurPos);
			INiceNet::Instance().SendMsg(m_ConID,pMsg,pMsg->GetLen() );
			return 0;
		}


		string CNHttpReq::GetParam( string strKey )
		{
			NMAP_STR::iterator it = m_MapStrParam.find( strKey );
			if( it==m_MapStrParam.end() )
				return "";
			return it->second;
		}
		string CNHttpReq::GetParamByIndex( int iIndex,string &strKey )
		{
			strKey.empty();
			if( (int)m_MapStrParam.size()<(iIndex+1) )
				return "";

			NMAP_STR::iterator it = m_MapStrParam.begin();
			for( int t=0;it!=m_MapStrParam.end(); t++,it++ )
			{
				if( t==iIndex )
				{
					strKey = it->first;
					return it->second;
				}
			}
			return "";
		}

        bool CNHttpReq::ClientIsPostman()
        {
            //strUAger = "PostmanRuntime/7.26.8"
            string strUAger = m_BaseInfo.User_Agent;
            size_t iFindPos = strUAger.find("PostmanRuntime");
            if( iFindPos != string::npos )
                return true;
            return false;
        }

        EmHttpJudge CNHttpReq::OnRevMsg(tcp_pak_header &recHeader)
		{
			if( m_BaseInfo.RecBuf_DataLen>0 )
				return EHttpJ_ErrOneDowith;

			if( recHeader.GetLen()>19535 )
			{
				int iss = 0;
				iss++;
			}

			int iErrID = CheckRecDataIsRight(recHeader );
			if( iErrID>0 )
			{
				NETLOG_ERROR("iErrID="<<iErrID<<FUN_FILE_LINE );
				return EHttpJ_CheckSaveFail;
			}
			boost::thread::id myTID = boost::this_thread::get_id();
			ostringstream strOTID; 
			strOTID<<myTID;
			CInPubFun::strcpy( m_BaseInfo.cThreadID,sizeof(m_BaseInfo.cThreadID),strOTID.str().c_str(),423401 );

			char strHBSplite[10]  = "\r\n\r\n";
			if( recHeader.GetLen()>1024*1024*100 ) //100M 最大了
				return EHttpJ_RecMsgTooLong;
			char *pHttp = m_pBufBeginPos+sc_pak_header_len;
	
			//判断整个请求是否发送数据完成

			char *pReqBody  = std::strstr( pHttp, strHBSplite );
			if( 0==pReqBody )
				return EHttpJ_ContinueRead;

			//整个请求的数据已经发送完成了

			pReqBody          += strlen(strHBSplite);
			unsigned iHeadLen  = 0;
			if( strlen(pHttp)>strlen(pReqBody) )
			{//打印头部信息
				char tempAss[2048]={0};
				iHeadLen = (unsigned)(pReqBody-pHttp);
				if( iHeadLen+10>sizeof(tempAss) ){
					return EHttpJ_HeadInfoTooMax;
				}

				memcpy(tempAss,pHttp,iHeadLen );
				int iErrPos = AnaHttp_Head_AllLine( tempAss );
				if( iErrPos>0 )
					return EHttpJ_HeadInfo_AnaErr;
			}else{
				return EHttpJ_LogicErrA;
			}

			if( EHttpReq_Get==m_BaseInfo.reqType )
			{
				m_BaseInfo.RecBuf_DataLen = m_Head.GetLen();
				return EHttpJ_OneReqOver;
			}
			else if( EHttpReq_Post==m_BaseInfo.reqType )
			{
				m_BaseInfo.PostDataBeginPos = iHeadLen+sc_pak_header_len;
				unsigned iPostDataLen = recHeader.GetLen()-sc_pak_header_len-iHeadLen;

				//NETLOG_SYSINFO("recHeader.Len="<<recHeader.Len<<"; Content_Length="<<m_BaseInfo.Content_Length );
				if( iPostDataLen == m_BaseInfo.Content_Length )
				{
					m_BaseInfo.RecBuf_DataLen = m_Head.GetLen();
					AnaPost_HttpHead_AllLine( pReqBody,iPostDataLen );
					return EHttpJ_OneReqOver;
				}
				else if( iPostDataLen<m_BaseInfo.Content_Length )
					return EHttpJ_ContinueRead;
				else
					return EHttpJ_PostDataLenErr;
			}else
				return EHttpJ_ErrNoGetPost;
			return EHttpJ_ErrP;
		}

		int CNHttpReq::AnaHttp_Head_AllLine( char *pHeadInfo )
		{
			if( m_BaseInfo.IsAnsHeadOver )
				return 0;

			//NETLOG_SYSINFO(pHeadInfo);
			NVEC_STR vAllLine;
			CInPubFun::SpliteStrToA(vAllLine,pHeadInfo,"\r\n");
			if( vAllLine.size()<1 )
				return 2310101;

			char chName[128] = {0};
			for(NVEC_STR::iterator it=vAllLine.begin();it!=vAllLine.end(); it++ )
			{
				char *pStrLine = (char*)it->c_str();
				if( strlen(pStrLine)>1024 )
				{
					NETLOG_ERROR(FUN_LINE<<" aabb:"<<pStrLine );
					continue;
				}

				char *pConLen  = std::strstr( pStrLine," ");
				if( (0==pConLen) || ((strlen(pConLen)+2)>strlen(pStrLine)) )
				{
					NETLOG_ERROR(FUN_LINE<<" aadd:"<<pStrLine );
					continue;
				}

				memset( chName,0,sizeof(chName) );
				if( strlen(pStrLine)-strlen(pConLen)+10>sizeof(chName) )
				{
					NETLOG_ERROR(FUN_LINE<<" aaff:"<<pStrLine );
					continue;
				}
				memcpy(chName,pStrLine,strlen(pStrLine)-strlen(pConLen) );
				pConLen += 1; //跳过空格第一个空格分隔符
				
				string strName = chName;
				string strData = pConLen;
				boost::trim( strName );
				boost::to_lower( strName );
				boost::trim( strData );

				if( string("post")==strName )
				{
					m_BaseInfo.reqType = EHttpReq_Post;
					NVEC_STR vTempStr;
					CInPubFun::SpliteStrToB(vTempStr,strData," " );
					if( vTempStr.size()>0 )
						CInPubFun::strcpy( m_BaseInfo.Interface,sizeof(m_BaseInfo.Interface),vTempStr[0].c_str(),5423501 );
				}else if( string("get")==strName )
				{
					m_BaseInfo.reqType = EHttpReq_Get;
					NVEC_STR vTempStr;
					CInPubFun::SpliteStrToMDes(vTempStr,strData,"? " );
					if( vTempStr.size()>0 ){
						CInPubFun::strcpy( m_BaseInfo.Interface,sizeof(m_BaseInfo.Interface),vTempStr[0].c_str(),5423501 );
					}

					if( vTempStr.size()>1){
						CHAnGetParam getParam( vTempStr[1].c_str(),(unsigned)vTempStr[1].size() );
						getParam.Dowith( m_MapStrParam );
					}
				}else if( string("host:")==strName )
				{
					CInPubFun::strcpy(m_BaseInfo.Host,sizeof(m_BaseInfo.Host),strData.c_str(),342401 );
				}else if( string("user-agent:")==strName )
				{
					CInPubFun::strcpy(m_BaseInfo.User_Agent,sizeof(m_BaseInfo.User_Agent),strData.c_str(),342405 );
				}
				else if( string("content-length:")==strName )
				{
					try{
						m_BaseInfo.Content_Length = atoi( strData.c_str() );
					}catch (...){
						NETLOG_ERROR( "strData="<<strData<<FUN_FILE_LINE );
						return 2310102;
					}
				}else if( string("content-type:") == strName )
				{ 
					if( string::npos != strData.find("x-www-form-urlencoded") ){
						m_BaseInfo.Content_Type = EHContentType_UrlEncode;
					}else if( string::npos != strData.find("form-data") )
					{
						m_BaseInfo.Content_Type = EHContentType_FormData;

						NVEC_STR vCTFormData;
						CInPubFun::SpliteStrToA(vCTFormData,strData.c_str(),"boundary=");
						if( vCTFormData.size()>1 ){
							CInPubFun::strcpy( m_BaseInfo.Boundary,sizeof(m_BaseInfo.Boundary),vCTFormData[1].c_str(),735301 );
						}

					}
				}
			}
			m_BaseInfo.IsAnsHeadOver = true;
			return 0;
		}

		int CNHttpReq::OneReqOver( tcp_pak_header &recHeader )
		{
			//如果是post要保留数据信息
			//int iErrID = CheckRecDataIsRight(recHeader );
			//if( iErrID>0 )
			//	return iErrID;
			//memcpy( m_pBufBeginPos,&recHeader,sc_pak_header_len );
			return 0;
		}
		int CNHttpReq::OneReqBegin( unsigned conid,char *pBufBeginPos )
		{
			m_ConID        = conid;
			m_pBufBeginPos = pBufBeginPos;

			m_Head.reset();
			m_Head.SetCheckCode();
			m_Head.SetLen(sizeof(m_Head));
			m_Head.SetHttpReqIndex(m_Index++);
			m_Head.SetMsgID(9999999);
			return 0;
		}
		int CNHttpReq::CheckRecDataIsRight( tcp_pak_header &recBufHeader )
		{
			if( recBufHeader.GetHttpReqIndex() != m_Head.GetHttpReqIndex() )
				return 562101;

			tcp_pak_header *pBHead = (tcp_pak_header*)m_pBufBeginPos;
			if( pBHead->GetHttpReqIndex() != recBufHeader.GetHttpReqIndex() )
				return 562103;
			if(pBHead->GetLen()<sizeof(m_Head) )
				return 562105;
			if(pBHead->GetLen()>1024*1024 )
				return 562107;
			m_Head.SetLen( recBufHeader.GetLen() );
			return 0;
		}

		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		int CNHttpReq::AnaPost_HttpHead_AllLine( char *pReqBody,unsigned iPostDataLen )
		{
			if( EHContentType_UrlEncode==m_BaseInfo.Content_Type )
			{
				string strSour = pReqBody;
				string strDes  = CUrlEncode::UrlDecode( strSour );
				TwoFun_AnaPostDataToParam( strDes.c_str() );
			}
			else if( EHContentType_FormData==m_BaseInfo.Content_Type )
			{
				CHFormData fromDd( pReqBody,iPostDataLen );
				int iErrID = fromDd.Dowith( m_MapStrParam );
				return iErrID;
			}
			else if( EHContentType_Binary==m_BaseInfo.Content_Type )
			{
			}

			return 0;
		}

		int CNHttpReq::TwoFun_AnaPostDataToParam( const char *pBodyBuf )
		{
			if( 0==pBodyBuf )
				return 0;
			NVEC_STR vGetData;
			CInPubFun::SpliteStrToA( vGetData,pBodyBuf,"&" );
			for( NVEC_STR::iterator it=vGetData.begin(); it!=vGetData.end(); it++ )
			{
				string strLine = *it;
				boost::trim( strLine );
				NVEC_STR vLines;
				CInPubFun::SpliteStrToB( vLines,strLine,"=" );
				if( vLines.size()<2 )
					continue;
				m_MapStrParam[ vLines[0] ] = vLines[1];
			}
			return 0;
		}

		unsigned CNHttpReq::GetPostBodyLen()
		{
			if( !m_BaseInfo.IsAnsHeadOver )
				return 0;
			return m_BaseInfo.Content_Length;
		}

		unsigned CNHttpReq::GetBodyNoReadLen()
		{
			if( false == this->IsPost() )
				return 0;

			if( false == m_BaseInfo.IsAnsHeadOver )
				return 0;
			if( m_BaseInfo.PostDataBeginPos<64)
				return 0;
			if( m_BaseInfo.Content_Length<(m_Head.GetLen()-m_BaseInfo.PostDataBeginPos) )
				return 0;
			return m_BaseInfo.Content_Length-(m_Head.GetLen()-m_BaseInfo.PostDataBeginPos);
		}

		////@@@@@@@@@@@@@@
	}
}
