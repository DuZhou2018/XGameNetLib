//	Octopus MMORPG Platform

#ifndef __OMP_NET_RSBUFFERSECOND_HEADER__
#define __OMP_NET_RSBUFFERSECOND_HEADER__

#include "../include/INiceLog.h"
#include "../header/CGlobal.hpp"

#include <boost/function.hpp>

namespace peony {
    namespace net
    {
		class TDebugS
		{
		public:
			unsigned short   wdMsgID;
			unsigned short   wdIndex;
		};

		/********************************************************************
			created:	2010/04/27	27:4:2010   10:40
			author:		zhanghongtao	
			purpose:	内存管理函数，实现灵活多变的内存分配方式
		*********************************************************************/
		typedef boost::function<bool (char *&buffer,unsigned usize,bool is_req_allocate, unsigned umark)>mghandler_buffer;

        static const unsigned sc_uBufferSizeDefault = 1024*32;
        //	buf_size:	tcp message pak including TDataHead struct
        template<class THEAD>
        class CBaseBuffer
        {
        protected:
            char *                  m_data;
            unsigned                m_PosHead;
            unsigned                m_PosTail;    //当前的尾部
            unsigned                m_PrePosTail; //是否应该掉头
            unsigned                sc_uHeadSize;
			unsigned                m_UID;        //为了调试 
			unsigned                m_UsedPercent;//缓冲区使用的万分数，
			mghandler_buffer        m_alloc;
        public:
            unsigned                m_BufSize;
        protected:
            CBaseBuffer(unsigned buf_size,mghandler_buffer alloc)
            {
				m_data       = 0; 
				m_alloc      = alloc;
				m_UsedPercent= 0;
				m_UID        = 0;
                sc_uHeadSize = sizeof(THEAD);
                m_BufSize    = buf_size;
                alloc(m_data,buf_size,true,0);
                this->reset();
            }
            virtual ~CBaseBuffer()
            {
				if( m_data )
					m_alloc(m_data,m_BufSize,false,0);
				m_data=0; 
			}

            void*           get_DataTailPtr()   { return ((char*)m_data + m_PosTail);     }
            void*           get_DataHeadPtr()   { return ((char*)m_data + m_PosHead);     }
            virtual void*   get_bodyPtr_length( unsigned data_len)
            {
                unsigned uLen = data_len;
                if(m_BufSize<=uLen)
                    return 0;

                if( 0!=m_PosHead && IsEmpty() )
                {
					reset();
                }

                if( m_PosTail>m_PosHead)
                {
                    unsigned iTailFreeLen = m_BufSize-m_PosTail;
                    unsigned iHeadFreeLen = m_PosHead;
                    if( iTailFreeLen>uLen)
                    {
                    }else if( iHeadFreeLen>uLen)
                    {//这里调头了
                        m_PrePosTail = m_PosTail;
                        m_PosTail    = 0;
                    }else
                        return 0;
                }else if( m_PosTail<m_PosHead )
                {
                    if( m_PosHead-m_PosTail<=uLen )
                        return 0;
                }
                return ( (char*)m_data + m_PosTail);
            }
            void            reset()
            {
                m_PosTail = m_PosHead=m_PrePosTail = 0;
				memset(m_data,0,m_BufSize);
            }

        public:
			bool            isok_alloc_buffer(){ return (0!=m_data);}
			unsigned        get_buffer_maxlen()const{ return m_BufSize; }

            bool            IsEmpty()       { return m_PosHead==m_PosTail?true:false; }
            void            pop()
            {
                if( this->IsEmpty() )
                    return;

                THEAD* pHeader = (THEAD*)((char*)m_data+m_PosHead);
                m_PosHead += (pHeader->GetLen()/*+sc_uHeadSize*/);
                if(m_PosHead>m_PosTail && m_PosHead==m_PrePosTail )
                {
                    if( !(m_PrePosTail>=m_PosHead) )
					{
						NETLOG_ERROR("[网络缓冲区管理错误]"<<FUN_FILE_LINE);
					}
                    m_PosHead    = 0;
                    m_PrePosTail = 0;
                }
            }
			void            SetDebugID(unsigned ID){m_UID=ID;}
			string          get_buffer_info()
			{
				unsigned datalen = 0;
				if( m_PosTail>m_PosHead)
				{
					datalen = m_PosTail-m_PosHead;
				}else if( m_PosTail<m_PosHead )
				{
					datalen = m_BufSize-(m_PosHead-m_PosTail);
				}

				ostringstream strBaseInfo;
				strBaseInfo<<"head="<<m_PosHead<<" tail="<<m_PosTail<<" len="<<datalen<<"  PrePosTail:"<<m_PrePosTail<<" Cent:"<<(int(datalen*100/m_BufSize))<<", buffersize="<<m_BufSize;
				return strBaseInfo.str();
			}
			void            debug_print_buffermsglist(unsigned ConID,vector<THEAD> &vecmsgheads )
			{
				unsigned dcur_PosHead = m_PosHead;
				unsigned maxloop      = 0;
				while(true)
				{
					++maxloop;
					if( maxloop>10000 )
					{
						NETLOG_ERROR("[网络低层，死循环] m_UID="<<m_UID<<FUN_FILE_LINE);
						break;
					}

					//如果为空
					if( dcur_PosHead==m_PosTail )
						return;
					if( dcur_PosHead>m_BufSize )
					{
						NETLOG_ERROR("[网络低层，debug错误] m_UID="<<m_UID<<";dcur_PosHead="<<dcur_PosHead<<";m_PosTail="<<m_PosTail<<FUN_FILE_LINE);
						return;
					}

					THEAD* pHeader = (THEAD*)((char*)m_data+dcur_PosHead);
					vecmsgheads.push_back( *pHeader );

					dcur_PosHead += pHeader->GetLen();
					if(dcur_PosHead>m_PosTail && (dcur_PosHead==m_PrePosTail) )
					{
						dcur_PosHead = 0;
					}
				}
			}
		protected:
			unsigned get_buffer_usedpercent()
			{
				unsigned datalen = 0;
				if( m_PosTail>m_PosHead)
				{
					datalen = m_PosTail-m_PosHead;
				}else if( m_PosTail<m_PosHead )
				{
					datalen = m_BufSize-(m_PosHead-m_PosTail);
				}
				m_UsedPercent = datalen*10000/m_BufSize;
				return m_UsedPercent;
			}
        };

        template<class THEAD>
        class CSendBuffer : public CBaseBuffer<THEAD>
        {
        private:
			unsigned  m_uSendOkCount;   //数据进入缓冲区就算成功,发送成功的数据包数
			unsigned  m_uSendFailCount; //发送失败
        public:
            CSendBuffer(mghandler_buffer alloc,unsigned buf_size=sc_uBufferSizeDefault):CBaseBuffer<THEAD>(buf_size,alloc)
            { m_uSendOkCount = m_uSendFailCount =0; }
            virtual ~CSendBuffer()
            {  }

            bool  push(const void *data_ptr, unsigned data_len)
            {
				//assert(data_len>0);
				if( 0==data_len )
				{
					NETLOG_ERROR(FUN_FILE_LINE);
					return true;
				}

                if( data_len<=0 )
                    return false;

                //TDataHead  Header;
                //Header.reset();
                //Header.length = data_len;
                void *pFreeData = this->get_bodyPtr_length( data_len/*+sc_uHeadSize*/ );
                if( pFreeData )
                {
                    //memcpy(pFreeData,&Header,sc_uHeadSize);
                    memcpy( (char*)pFreeData/*+sc_uHeadSize*/,data_ptr,data_len );
					//TDebugS *pDebugMsg = (TDebugS*)(data_ptr);
					//pDebugMsg = (TDebugS*)(m_data+m_PosTail+sc_uHeadSize);
                    CBaseBuffer<THEAD>::m_PosTail += (data_len/*+sc_uHeadSize*/);

					m_uSendOkCount++;
                    return true;
                }
				m_uSendFailCount++;
                return false;
            }
            void* front(unsigned &data_len)
            {
                if( this->IsEmpty() )
                    return 0;
				//TDebugS *pDebugMsg = (TDebugS*)(m_data+m_PosHead+sc_uHeadSize);
				THEAD* pHeader = (THEAD*)((char*)CBaseBuffer<THEAD>::m_data+CBaseBuffer<THEAD>::m_PosHead);
                //assert(pHeader->check());
				if( !pHeader->check() )
				{
					NETLOG_ERROR(FUN_FILE_LINE);
				}

                data_len = pHeader->GetLen()/*+sc_uHeadSize*/;
                return (char*)CBaseBuffer<THEAD>::m_data+CBaseBuffer<THEAD>::m_PosHead;
            }

			//debug info
			/********************************************************************
				created:	2009/11/12	12:11:2009   10:41
				author:		zhanghongtao	
				purpose:	返回使用的百分比
			*********************************************************************/
			unsigned get_sendbuffer_info(unsigned &usendokcount,unsigned &usendfailcount)
			{
				usendokcount   = m_uSendOkCount;
				usendfailcount = m_uSendFailCount;
				return CBaseBuffer<THEAD>::get_buffer_usedpercent();
			}
			void clear_send_counterinfo()
			{
				m_uSendFailCount = m_uSendOkCount = 0;
			}
        };

        template<class THEAD>
        class CReciveBuffer : public CBaseBuffer<THEAD>
        {
        public:
            bool            m_IsStopRec;
            bool            m_pak_header_readed;
            THEAD           m_RHeader;
		private:
			unsigned        m_uReciveCount;  //收到的数据包数目
        public:
            CReciveBuffer(mghandler_buffer alloc,unsigned buf_size=sc_uBufferSizeDefault):CBaseBuffer<THEAD>(buf_size,alloc)
            {
				m_uReciveCount      = 0;
                m_IsStopRec         = false;
                m_pak_header_readed = false;
            }
            virtual ~CReciveBuffer()
            {}
            void *open_for_push( unsigned data_len )
            {
                void *pFreeData = this->get_bodyPtr_length( data_len );
                if( pFreeData )
                {
                    return pFreeData;
                }
                return 0;
            }
            bool finish_push(unsigned data_len)
            {
                if( data_len<=0 )
                    return true;

                THEAD *pHead = (THEAD*)this->get_DataTailPtr();
                if(    pHead->check() 
                    && m_RHeader.check()
                    && (pHead->GetLen()==m_RHeader.GetLen())
                    && (pHead->GetLen()==data_len)
                   )
                {
                    CBaseBuffer<THEAD>::m_PosTail += data_len;
					m_uReciveCount++;
                    return true;
                }else
                {
					NETLOG_ERROR("Logic error....(1), please close recive finish length!=need recive length" << FUN_LINE );
					NETLOG_ERROR("    [ checkAA=" << pHead->check() << ", checkBB=" << m_RHeader.check() << FUN_LINE);
					NETLOG_ERROR("    [ data_len=" << data_len << " pGetLen=" << pHead->GetLen() << ", RGetLen=" << m_RHeader.GetLen() << FUN_LINE);
					if (false == pHead->check())
					{
						NETLOG_ERROR("Init Param seting error, m_IsCheckPackHead(Please close) ....(1), please close recive finish length!=need recive length" << FUN_LINE);
					}
					return false;
                }
            }
            bool finish_push_Udp(unsigned data_len)
            {
                if(data_len<=0)
                    return true;

                THEAD *pHead = (THEAD*)this->get_DataTailPtr();
                if(pHead->check() && (pHead->GetLen()==data_len) )
                {
                    CBaseBuffer<THEAD>::m_PosTail += data_len;
                    m_uReciveCount++;
                    return true;
                }
                else
                {
                    //assert(0);
                    NETLOG_ERROR(__FUNCTION__<<"(2), recive finish length!=need recive length ");
                    return false;
                }
            }
            bool finish_push_http(unsigned data_len, bool isOver)
			{
				if( data_len<=0 )
					return true;

				if( m_RHeader.check() )
				{
					CBaseBuffer<THEAD>::m_PosTail += data_len;
					if( isOver )
						m_uReciveCount++;
					return true;
				}else
				{
					//assert(0);
					NETLOG_ERROR(__FUNCTION__<<"(3), recive finish length!=need recive length");
					return false;
				}
			}

            void * front(unsigned &data_len)
            {
                data_len = 0;
                if( this->IsEmpty() )
                    return 0;

                THEAD* pHeader = (THEAD*)((char*)CBaseBuffer<THEAD>::m_data+CBaseBuffer<THEAD>::m_PosHead);
				if( !pHeader->check() )
				{
					NETLOG_ERROR(FUN_FILE_LINE);
				}

                data_len = pHeader->GetLen();
                if( data_len>CBaseBuffer<THEAD>::m_BufSize )
                {
                    data_len = 0;
                    return 0;
                }
                return (char*)CBaseBuffer<THEAD>::m_data+CBaseBuffer<THEAD>::m_PosHead/*+sc_uHeadSize*/;
            }
			//debug info
			/********************************************************************
			created:	2009/11/12	12:11:2009   10:41
			author:		zhanghongtao	
			purpose:	返回使用的百分比
			*********************************************************************/
			unsigned get_recivebuffer_info(unsigned &reciveCount)
			{
				reciveCount   = m_uReciveCount;
				return CBaseBuffer<THEAD>::get_buffer_usedpercent();
			}
			void clear_recive_counterinfo()
			{
				m_uReciveCount = 0;
			}
			char *HttpGetBufBegin()
			{
				return (char*)CBaseBuffer<THEAD>::m_data+sizeof(m_RHeader);
			}
        };
    }	// end of namespace net
}	// end of namespace peony


#endif //#define __OMP_NET_RECVBUFFER_HEADER__