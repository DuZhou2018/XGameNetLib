//	Octopus MMORPG Platform
//
//	Copyright(c) by NineYou Information technology(Shanghai) Co., Ltd.
//	created by Wang Zhiyong, 2008


#ifndef __OMP_NET_RECVBUFFER_HEADER__
#define __OMP_NET_RECVBUFFER_HEADER__

#include <boost/array.hpp>
#include <vector>
#include "../header/TCPPakDef.hpp"

namespace peony {
    namespace net
    {
        //	buf_size:	tcp message pak including tcp_pak_header struct
        struct RecvBuffer
        {
        public:
            tcp_pak_header              m_Header;
        private:
            char *                    	m_data;
            unsigned int                m_PosHead;
            unsigned int                m_PosTail;
            unsigned int                m_BufSize;
            unsigned int                m_PrePosTail; //是否应该掉头
            bool                        m_IsFill;
        public:
            bool                        m_pak_header_readed;
        public:
            RecvBuffer(unsigned int buf_size=1024*64);
            virtual ~RecvBuffer();

            bool            IsEmpty();

            void*           open_for_push(unsigned int data_len=0 );
            void            finish_push(unsigned int data_len=0);

            void*           front(unsigned int &data_len){ return ((char*)m_data + m_PosHead);}
            void            pop();
        private:
            void  PushNetHeadToBody();
            void  reset();

        };
    }	// end of namespace net
}	// end of namespace peony


#endif //#define __OMP_NET_RECVBUFFER_HEADER__