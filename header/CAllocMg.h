/****************************************************************************************************
	created:	2010-8-25   15:46
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\header\CAllocMg.h
	author:		Zhanghongtao
	
	purpose:	
*****************************************************************************************************/
#ifndef sgs_server_CAllocMg_h__
#define sgs_server_CAllocMg_h__
#include "./InPublicPreDefine.hpp"

namespace peony
{
	namespace net
	{
		class CAllocItem
		{
			typedef std::map<char *,unsigned short> MAP_MYITEM;
			typedef std::list<char *>               LIST_MYFREE;
		private:
			friend class CAllocMg;
			CAllocItem(unsigned ubuffersize,unsigned umaxcount);
		public:
			~CAllocItem(void);
		public:
			bool alloc_buffer(char *&buffer);
			bool free_buffer( char *&buffer);
		private:
			void add_mem();
		private:
			MAP_MYITEM		m_myitem;
			LIST_MYFREE		m_free;
			unsigned	m_buffersize;
			unsigned	m_maxcount;
		};

		class CAllocMg
		{
			typedef map<unsigned,CAllocItem *>MAP_ALLOCITEM;
		private:
			friend class PNGB;
			CAllocMg(void);
		public:
			~CAllocMg(void);
		public:
			int  add_alloc(unsigned usize, unsigned umax_count);
			bool alloc_buffer(char *&buffer,unsigned usize);
			bool free_buffer( char *&buffer,unsigned usize);
		private:
			MAP_ALLOCITEM           m_allocs;
			boost::recursive_mutex &m_mutex;
		};
	}
}


#endif // sgs_server_CAllocMg_h__
