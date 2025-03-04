/****************************************************************************************************
	created:	2010-8-24   10:30
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\header\NiceNet.h
	author:		Zhanghongtao
	
	purpose:	
	_STLP_USE_STATIC_LIB,使用stlport静态库
	存在问题：[1] "ec.message()"当有这个对象的调用的时候程序会僵死。估计是内存溢出!，好像在stlport里面有.
				  这个问题我初步的估计是因为boost的库编译的时候部分模块没有使用stlport的string而造成的.
*****************************************************************************************************/
#ifndef NICE_NET_IMP_HTTPREQ_IMP
#define NICE_NET_IMP_HTTPREQ_IMP
#include "../include/IHttpReq.h"
#include "../header/InPublicPreDefine.hpp"
#include "./TCPPakDef.hpp"

namespace peony
{
    namespace net
    {
		enum EmHttpJudge
		{
			EHttpJ_NoDef = 0,
			EHttpJ_OneReqOver,			//一个请求完成
			EHttpJ_ContinueRead,		//继续读数据
			EHttpJ_LogicErrA,			//数据接收完成后，处理数据逻辑错误A
			EHttpJ_HeadInfoTooMax,		//头部信息大于缓冲区长度
			EHttpJ_HeadInfo_AnaErr,		//解析头部信息失败
			EHttpJ_PostDataLenErr,

			EHttpJ_CheckSaveFail,
			EHttpJ_RecMsgTooLong,		//发送了错误
			EHttpJ_FindFirstLineErr,	//发送了错误
			EHttpJ_FirstLineLenErr,
			EHttpJ_FirstLineMaxErr,
			EHttpJ_FirstLineSpliteErr,
			EHttpJ_ErrNoGetPost,
			EHttpJ_ErrP,
			EHttpJ_ErrOneDowith,
		};

		enum EmHttpReqType
		{
			EHttpReq_NoDef = 0,
			EHttpReq_Get,
			EHttpReq_Post,
		};
		enum EmHttpContentType
		{
			EHContentType_NoDef = 0,
			EHContentType_UrlEncode,
			EHContentType_FormData,
			EHContentType_Binary,
		};

		struct TBaseInfo
		{
			char                cThreadID[32];
			bool                IsAnsHeadOver;      //http头部是否已经解析完成,用来标记，不要重复处理
			EmHttpReqType		reqType;            //请求类型,get, post,
			char				Interface[128];		//接口
			unsigned			Content_Length;		//post数据的长度
			unsigned            RecBuf_DataLen;     //接收缓冲区里面数据的长度,所有的一个http请求的所有数据
			EmHttpContentType	Content_Type;		//传输数据的格式
			char                Boundary[64];
			char				Host[32];			//客户端ip地址
			char                User_Agent[256];    //

			unsigned            PostDataBeginPos;	//post原始数据的开始位置,现在这个位置其实就是接收缓冲区的头部+http头部的位置

			//rep seting
			char				Rep_ContentType[32];//返回的数据格式	
		};

        class CNHttpReq : public IHttpReq
        {
		public:
            CNHttpReq(void);
            virtual ~CNHttpReq(void);

		public: //接口部分,对外逻辑层的接口
			virtual unsigned    GetConnectID(){ return m_ConID; }
			virtual string		GetClientIp(){ return string(m_BaseInfo.Host); }
			virtual int  		GetParamCount(){ return (int)m_MapStrParam.size(); }
			virtual string		GetParam( string strKey );
			virtual string      GetParamByIndex( int iIndex,string &strKey);
			virtual string		GetMethod(){ return string(m_BaseInfo.Interface); }
			virtual bool		IsPost(){ return m_BaseInfo.reqType==EHttpReq_Post; }
			virtual bool		IsGet(){  return m_BaseInfo.reqType==EHttpReq_Get; }
			virtual int			RepMsg( const char *pRepData,unsigned iDataLen );
			virtual const char*	GetPostData( unsigned &uDataLen );
			virtual void		SetRepContentType( string strCType );
			virtual string      Debug_GetNThreadID(){ return string(m_BaseInfo.cThreadID); }
			virtual unsigned    Stat_GetRecSize(){ return m_BaseInfo.RecBuf_DataLen; }
            virtual bool        ClientIsPostman();

		public: //下面的接口是HttpSession用来完成数据的接收
			EmHttpJudge		OnRevMsg( tcp_pak_header &recHeader );
			int 			OneReqBegin( unsigned conid, char *pBufBeginPos );
			int				OneReqOver( tcp_pak_header &recHeader );
			tcp_pak_header  GetMsgHead(){ return m_Head; }

			//如果是post方式，当头部接收完成后，返回消息体的数据长度
			unsigned        GetPostBodyLen();
			unsigned        GetBodyNoReadLen();

		private:
			//解析整个头部信息
			int     		AnaHttp_Head_AllLine( char *pHeadInfo );

			//解析整个头部信息
			int      		AnaPost_HttpHead_AllLine( char *pReqBody,unsigned iPostDataLen );
			//获取post的数据参数
			int				TwoFun_AnaPostDataToParam(const char *pBodyBuf);
			//获取get的数据参数
			int				CheckRecDataIsRight(tcp_pak_header &recBufHeader);

			//生成Rep的头部信息
			string			InFun_BuildHttpHead();

		private:
			unsigned        m_ConID;		//用来返回消息用
			unsigned	    m_Index;		//请求顺序，内部生成
			char		   *m_pBufBeginPos;	//在接收缓冲区，这个请求开始的位置


			TBaseInfo       m_BaseInfo;
			NMAP_STR        m_MapStrParam;	//接收到个标准数据，解析后放到这里
			tcp_pak_header	m_Head;
        };
    }
}
#endif

