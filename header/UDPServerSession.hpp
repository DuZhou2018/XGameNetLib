/***************************************************************************************************************************************
	author:	    DuZhou   qq: 1265008170,   mail: zht213@163.com
	created:    2021/9/28 11:36
	filename:   UDPServerSession.hpp
	purpose:	
***************************************************************************************************************************************/
#ifndef UDPServerSession_h__
#define UDPServerSession_h__
#include "./INiceNetServer.h"

namespace peony {
	namespace net {		

		class UDPServerSession : public INiceNetSocket
		{
		public:			
			UDPServerSession(   io_context            &io_service,
								TXZTBaseInitPm         &initPM,
								const TNewServerParam  &param);
			~UDPServerSession();

		public: //INiceNetSocket 接口部分
			virtual void	StartSocket();
			virtual bool	Run(unsigned uCount);
			virtual void	CloseSocketOut(string strwhy);

			int 			SendMsg( const void* data_ptr, unsigned data_size );
            void            NtfAppConnect( INetSocketPtr pSelf );
            void            PushRecOneMsg( char *pBuff,unsigned iDataSize );

		private:
			virtual bool    DoNetInCmd(unsigned connectid,const void*pdata,unsigned data_len);

		public:
			const TNewServerParam  &m_param;
            udp::endpoint	        m_mepoint;

        private:
			INetSocketPtr           m_pSelfAa;
            boost::recursive_mutex	m_recvbuf_mutex;		 //	for m_recv_buf's res. protection
            SendBufferType			m_recv_buf;
        };

	}	// end of namespace net

}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
