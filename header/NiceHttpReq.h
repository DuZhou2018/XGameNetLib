/****************************************************************************************************
	created:	2010-8-24   10:30
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\header\NiceNet.h
	author:		Zhanghongtao
	
	purpose:	
	_STLP_USE_STATIC_LIB,ʹ��stlport��̬��
	�������⣺[1] "ec.message()"�����������ĵ��õ�ʱ�����Ὡ�����������ڴ����!��������stlport������.
				  ��������ҳ����Ĺ�������Ϊboost�Ŀ�����ʱ�򲿷�ģ��û��ʹ��stlport��string����ɵ�.
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
			EHttpJ_OneReqOver,			//һ���������
			EHttpJ_ContinueRead,		//����������
			EHttpJ_LogicErrA,			//���ݽ�����ɺ󣬴��������߼�����A
			EHttpJ_HeadInfoTooMax,		//ͷ����Ϣ���ڻ���������
			EHttpJ_HeadInfo_AnaErr,		//����ͷ����Ϣʧ��
			EHttpJ_PostDataLenErr,

			EHttpJ_CheckSaveFail,
			EHttpJ_RecMsgTooLong,		//�����˴���
			EHttpJ_FindFirstLineErr,	//�����˴���
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
			bool                IsAnsHeadOver;      //httpͷ���Ƿ��Ѿ��������,������ǣ���Ҫ�ظ�����
			EmHttpReqType		reqType;            //��������,get, post,
			char				Interface[128];		//�ӿ�
			unsigned			Content_Length;		//post���ݵĳ���
			unsigned            RecBuf_DataLen;     //���ջ������������ݵĳ���,���е�һ��http�������������
			EmHttpContentType	Content_Type;		//�������ݵĸ�ʽ
			char                Boundary[64];
			char				Host[32];			//�ͻ���ip��ַ
			char                User_Agent[256];    //

			unsigned            PostDataBeginPos;	//postԭʼ���ݵĿ�ʼλ��,�������λ����ʵ���ǽ��ջ�������ͷ��+httpͷ����λ��

			//rep seting
			char				Rep_ContentType[32];//���ص����ݸ�ʽ	
		};

        class CNHttpReq : public IHttpReq
        {
		public:
            CNHttpReq(void);
            virtual ~CNHttpReq(void);

		public: //�ӿڲ���,�����߼���Ľӿ�
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

		public: //����Ľӿ���HttpSession����������ݵĽ���
			EmHttpJudge		OnRevMsg( tcp_pak_header &recHeader );
			int 			OneReqBegin( unsigned conid, char *pBufBeginPos );
			int				OneReqOver( tcp_pak_header &recHeader );
			tcp_pak_header  GetMsgHead(){ return m_Head; }

			//�����post��ʽ����ͷ��������ɺ󣬷�����Ϣ������ݳ���
			unsigned        GetPostBodyLen();
			unsigned        GetBodyNoReadLen();

		private:
			//��������ͷ����Ϣ
			int     		AnaHttp_Head_AllLine( char *pHeadInfo );

			//��������ͷ����Ϣ
			int      		AnaPost_HttpHead_AllLine( char *pReqBody,unsigned iPostDataLen );
			//��ȡpost�����ݲ���
			int				TwoFun_AnaPostDataToParam(const char *pBodyBuf);
			//��ȡget�����ݲ���
			int				CheckRecDataIsRight(tcp_pak_header &recBufHeader);

			//����Rep��ͷ����Ϣ
			string			InFun_BuildHttpHead();

		private:
			unsigned        m_ConID;		//����������Ϣ��
			unsigned	    m_Index;		//����˳���ڲ�����
			char		   *m_pBufBeginPos;	//�ڽ��ջ��������������ʼ��λ��


			TBaseInfo       m_BaseInfo;
			NMAP_STR        m_MapStrParam;	//���յ�����׼���ݣ�������ŵ�����
			tcp_pak_header	m_Head;
        };
    }
}
#endif

