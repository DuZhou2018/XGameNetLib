#include "../header/CAllocMg.h"
#include "../include/INiceLog.h"
#include "../header/CGlobal.hpp"

namespace peony
{
	namespace net 
	{
		CAllocMg::CAllocMg( void ):m_mutex(*PNGB::m_globMutex)
		{

		}

		CAllocMg::~CAllocMg( void )
		{
			MAP_ALLOCITEM::iterator it = m_allocs.begin();
			for( ;it!=m_allocs.end(); ++it )
			{
				CAllocItem *pitem = it->second;
				delete pitem;
			}
			m_allocs.clear();
		}

		bool CAllocMg::alloc_buffer( char *&buffer,unsigned usize )
		{
			Boost_Scoped_Lock NiceNet_lock(m_mutex);

			MAP_ALLOCITEM::const_iterator it = m_allocs.find( usize );
			if( m_allocs.end()==it )
			{
				NETLOG_ERROR("usize="<<usize<<FUN_FILE_LINE);
				return false;
			}
			return it->second->alloc_buffer(buffer);
		}

		bool CAllocMg::free_buffer( char *&buffer,unsigned usize )
		{
			Boost_Scoped_Lock NiceNet_lock(m_mutex);

			MAP_ALLOCITEM::iterator it = m_allocs.find( usize );
			if( m_allocs.end() == it )
			{
				NETLOG_ERROR("usize="<<usize<<FUN_FILE_LINE);
				return false;
			}
			return it->second->free_buffer(buffer);
		}

		int CAllocMg::add_alloc( unsigned usize, unsigned umax_count )
		{
			Boost_Scoped_Lock NiceNet_lock(m_mutex);

			MAP_ALLOCITEM::iterator it = m_allocs.find( usize );
			if( m_allocs.end() != it )
			{
				NETLOG_ERROR("usize="<<usize<<FUN_FILE_LINE);
				return -1;
			}
			CAllocItem *pitem = new CAllocItem(usize,umax_count);
			if( pitem )
			{
				m_allocs[usize] = pitem;
				return 0;
			}
			return 1;
		}
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CAllocItem::CAllocItem( unsigned ubuffersize,unsigned umaxcount )
		{
			m_buffersize = ubuffersize;
			m_maxcount   = umaxcount;
			NETLOG_NORMAL("ubuffersize="<<ubuffersize<<";umaxcount"<<umaxcount<<FUN_FILE_LINE);
		}

		CAllocItem::~CAllocItem( void )
		{
			MAP_MYITEM::iterator it = m_myitem.begin();
			for( ;it!=m_myitem.end(); ++it )
			{
				char *pitem = it->first;
				delete[] pitem;
			}
			m_myitem.clear();
		}

		bool CAllocItem::alloc_buffer( char *&buffer )
		{
			if( 0==m_free.size() )
				this->add_mem();
			if( 0==m_free.size() )
				return false;
			buffer = m_free.front();
            m_free.pop_front();

            memset( buffer,0, m_buffersize );
			return true;
		}

		bool CAllocItem::free_buffer( char *&buffer )
		{
			MAP_MYITEM::const_iterator it = m_myitem.find(buffer);
			if( it == m_myitem.end() )
			{
				NETLOG_ERROR("m_free.size="<<m_free.size()<<FUN_FILE_LINE);
				return false;
			}
			m_free.push_back( buffer );
			return true;
		}

		void CAllocItem::add_mem()
		{
			if( m_myitem.size()>=m_maxcount )
				return;
			unsigned ucount = m_maxcount/20;
			ucount              = (ucount==0)?1:ucount;
			for( unsigned t=0; t<ucount; ++t )
			{
				char *buffer = new (std::nothrow) char[m_buffersize];
				if( 0==buffer )
				{
					NETLOG_ERROR(FUN_FILE_LINE);
					return;
				}
				m_myitem[buffer] = 'a';
				m_free.push_back( buffer );
			}
		}
	}
}