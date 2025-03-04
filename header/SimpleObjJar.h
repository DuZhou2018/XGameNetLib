#ifndef _PEYONENET_SIMPLEOBJJAR_ZHT
#define _PEYONENET_SIMPLEOBJJAR_ZHT
#include "./InPublicPreDefine.hpp"

namespace peony
{
    namespace net
    {
        //获得和释放的时候不执行构造函数
        template <typename T>class CObjMg
        {
        private:
            typedef list<T*> LIST_SIMPLEJAR;
        protected:
            CObjMg(void)
            {
                m_uTotalCount = 0;
            }
            ~CObjMg(void)
            {
               UnInit();
            }
        public:
            void UnInit()
            {
                Boost_Scoped_Lock recvbuf_lock(m_locker_mutex);

				while( !m_listAll.empty() )
				{
					T * pItem = (T*)( *m_listAll.begin() );
					char *pt = static_cast<char *>(((void*)pItem));
					delete []pt;
					m_listAll.pop_front();
				}
                m_listAll.clear();
                m_listFree.clear();
            }
            T* malloc()
            {
				if( 0==m_uTotalCount )
				{
					return 0;
				}
                T* pObj = 0;
                Boost_Scoped_Lock recvbuf_lock(m_locker_mutex);
                if( !m_listFree.empty() )
                {
                    pObj = m_listFree.front();
                    m_listFree.pop_front();
                }
                return pObj;
            }
            void free(T* pObj)
            {
                Boost_Scoped_Lock recvbuf_lock(m_locker_mutex);
                m_listFree.push_front( pObj ); //提高速度，	
			}
        public:
            unsigned GetFreeCount(void)
            {
                size_t n = 0;
                Boost_Scoped_Lock recvbuf_lock(m_locker_mutex);
                n = m_listFree.size();
                return (unsigned)n;
            }
            unsigned GetAllCount()
            {
                return (unsigned)m_listAll.size();
            }
            long GetSimpleObjMemSize()
            {
                return (long)(m_uTotalCount*sizeof(T));
            }
		protected:
			void allloc( unsigned count )
			{
				Boost_Scoped_Lock recvbuf_lock(m_locker_mutex);
				for( unsigned t=0;t<count; ++t )
				{
					T* pNew   = (T*)((void*)(new (std::nothrow) char[sizeof(T)]));
					if( !pNew )
					{
						return;
					}	

					m_listFree.push_back( pNew );
					m_listAll.push_back( pNew );
					m_uTotalCount++;
				}
				return;
			}
        protected:
            LIST_SIMPLEJAR		    m_listFree;
			LIST_SIMPLEJAR		    m_listUseing;
            LIST_SIMPLEJAR		    m_listAll;
            unsigned		    m_uTotalCount; //cur count
            boost::recursive_mutex	m_locker_mutex;
        };

        template <typename T,bool IsCall=false>class CSimpleObjJar: public CObjMg<T>
        {
        private:
            bool m_IsCall;
        public:
            CSimpleObjJar(void)
            {
                m_IsCall      = IsCall;
            }
            ~CSimpleObjJar(void)
            {
            }
            virtual bool Init( unsigned dwCount )
            {
                Boost_Scoped_Lock recvbuf_lock(CObjMg<T>::m_locker_mutex);
                if( CObjMg<T>::m_uTotalCount>0 )
                    return true;
				CObjMg<T>::allloc( dwCount );
                return true;
            }
            virtual T* FetchObj(void)
            {
				T *pRet = CObjMg<T>::malloc();
                if(m_IsCall &&pRet )
                    new(pRet) T;
                return pRet;
            }
            virtual void ReleaseObj(T* pObj)
            {
                if( m_IsCall )
                    pObj->~T();
				CObjMg<T>::free(pObj);
            }
        };

        template <typename T>class _Jar_TCPServerSession : public CObjMg<T>
        {
        public:
            _Jar_TCPServerSession(void)
            {
            }
            ~_Jar_TCPServerSession(void)
            {
            }
			virtual void ReleaseObj(T* pObj)
			{
				if( pObj )
				{
					pObj->~T();
				}
				CObjMg<T>::free(pObj);
			}

            bool SetJarInitParam( unsigned dwCountMax )
            {
                Boost_Scoped_Lock recvbuf_lock(CObjMg<T>::m_locker_mutex);
                m_iMaxCount = dwCountMax;
                return true;
            }
            bool AddNewObj( unsigned add_count )
            {
                Boost_Scoped_Lock recvbuf_lock(CObjMg<T>::m_locker_mutex);
                if( CObjMg<T>::m_uTotalCount>=m_iMaxCount )
                    return false;
				if( CObjMg<T>::m_uTotalCount+add_count>m_iMaxCount )
					add_count = m_iMaxCount-CObjMg<T>::m_uTotalCount;
				CObjMg<T>::allloc( add_count );
                return true;
            }
            unsigned GetMayAddNewCount()
            {
                Boost_Scoped_Lock recvbuf_lock(CObjMg<T>::m_locker_mutex);
                return m_iMaxCount-CObjMg<T>::m_uTotalCount;
            }
        private:
            unsigned        m_iMaxCount;
        };

    }
}
#endif
