/********************************************************************
	created:	2:7:2010   16:29
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\include\
	author:		Zhanghongtao
	
	purpose:	提供网络服务器，只支持tcp  http请求

*********************************************************************/
#ifndef _INICE_NET_HTTPREQ_
#define _INICE_NET_HTTPREQ_

#include "./INiceLog.h"
namespace peony
{
    namespace net
    {
        class IHttpReq
        {
		protected:
            IHttpReq(){}
		public:
            virtual ~IHttpReq(){}

        public:
			virtual bool		IsPost()							=0;
			virtual bool		IsGet()								=0;
			virtual unsigned    GetConnectID()                      =0;
			virtual string		GetClientIp()						=0;
			virtual int 		GetParamCount()						=0;
			virtual string		GetParam( string strKey )			=0;
			virtual string      GetParamByIndex( int iIndex,string &strKey)= 0;
			virtual string		GetMethod()							=0;
			virtual const char*	GetPostData( unsigned &uDataLen )	=0; //获得post方式的原始数据
			virtual int			RepMsg( const char *pRepData,unsigned iDataLen ) = 0;
			virtual void		SetRepContentType( string strCType ) = 0;
			virtual unsigned    Stat_GetRecSize() =0;
			virtual string      Debug_GetNThreadID() = 0;
            virtual bool        ClientIsPostman(){ return false;}

			void                SetExData( unsigned v ){ m_uExData=v; }
			unsigned            GetExData(){ return m_uExData; }
			
		private:
			unsigned            m_uExData;
        };
	}
}

#endif
