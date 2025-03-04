//	Octopus MMORPG Platform
//
//	Copyright(c) by NineYou Information technology(Shanghai) Co., Ltd.
//	created by Wang Zhiyong, 2008

#ifndef __OMP_FIFO_BUFFER_HEADER_FILE__
#define __OMP_FIFO_BUFFER_HEADER_FILE__

#include <boost/array.hpp>
#include <boost/static_assert.hpp>

#include <limits>
#include <utility>
#include <cassert>

namespace peony {
	namespace net {

		//-------------------------------------------------
		//
		//	DIAGRAM of FifoBuffer Storage
		//
		//-------------------------------------------------
		//
		//	-> legend:
		//
		//		¡ò	for m_data head(m_data length)
		//		¡ö	for m_data body
		//		¡õ	for empty buffer
		//		¡ï	for reversed flag
		//
		//	-> normal status:
		//
		//		¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ		status when initialized
		//		head
		//		tail
		//
		//		¡ò¡ö¡ö¡ò¡ö¡ö¡ö¡ö¡ö¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ
		//		head              tail
		//
		//		¡õ¡õ¡õ¡õ¡ò¡ö¡ö¡ò¡ö¡ö¡ö¡ö¡ö¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ
		//		        head              tail
		//
		//		¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡ò¡ö¡ö¡ö¡ö¡ö¡õ
		//		                            head        tail
		//		
		//
		//	-> reversed status:
		//
		//		¡ò¡ö¡ö¡ö¡ö¡ö¡ö¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡ò¡ö¡ö¡ï¡õ¡õ¡õ		normal reversed status
		//		              tail          head
		//
		//		¡ò¡ö¡ö¡ö¡ö¡ö¡õ¡õ¡õ¡õ¡õ¡ò¡ö¡ö¡ö¡ò¡ö¡ï¡õ¡õ¡õ		normal reversed status
		//		              tail    head
		//
		//		¡ò¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ò¡ö¡ö¡õ¡õ¡õ¡ò¡ö¡ö¡ö¡ö¡ï¡õ		normal reversed status
		//		                      tail  head
		//
		//		¡ò¡ö¡ö¡ö¡ö¡ö¡ö¡ò¡ö¡ö¡õ¡õ¡õ¡õ¡õ¡õ¡ò¡ö¡ö¡ö¡ö		there is not places for 'reversed flag'
		//		                    tail        head
		//
		//		¡ò¡ö¡ö¡ò¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ò¡ö¡ö¡ö¡ö¡ö¡õ		ptr_head & ptr_tail are overlapped
		//		                            head
		//		                            tail
		//
		//	INFO:
		//		while 'reversed' is 'false', if left mem is not enough for push and 
		//			there is avalable mem at the front of the array, rewind the array
		//			and make 'reversed' become 'true'
		//
		//		while 'reversed' be 'true', ptr_head may be equal or larger than ptr_tail
		//

		using std::numeric_limits;

		typedef std::pair<void*,std::size_t>	fifo_buf_info;		// first: void*, second: size_t

		struct FifoBuffer
		{
        private:
			BOOST_STATIC_ASSERT( sizeof(char) == 1 );

			size_t	ptr_head;
			size_t	ptr_tail;
			bool	reversed;
            size_t  buffer_len;
			char*	m_data;

        public:
			FifoBuffer(unsigned int send_buf_size=1024*64) : ptr_head(0),ptr_tail(0),reversed(false)
            {
                buffer_len = send_buf_size;
                m_data = new char[send_buf_size];
            }
            ~FifoBuffer()
            {
                buffer_len = 0;
                delete [] m_data;
                m_data =0;
            }

            inline bool empty() const	{	return reversed ? false : ptr_head == ptr_tail;	}
			inline bool can_push(size_t size)
			{
				if( reversed )
				{
					return( ptr_head-ptr_tail > size+sizeof(size_t) );
				}
				else
				{
					if( buffer_len-ptr_tail>=size+sizeof(size_t) || ptr_head>=size+sizeof(size_t) )
						return true;
				}
				return false;
			}

			void pop()
			{
				if(empty())	return;

				if(!reversed)
				{
					ptr_head += *((size_t*)(m_data+ptr_head));
					ptr_head += sizeof( size_t );

					if(ptr_head==ptr_tail)
						ptr_head=ptr_tail=0;

					return;
				}

				//	reversed == true

				if(buffer_len-ptr_head<sizeof(size_t))
				{
					//rewind
					ptr_head = 0;					
					reversed = false;			// may be modified
					assert( ptr_tail > 0 );		// certainly! why it's reversed ? because we pushed something at the array's front pos.
				}
				else
				{
					size_t flag = *((size_t*)(m_data+ptr_head));
					if(flag == -1)
					{
						//rewind
						ptr_head = 0;					
						reversed = false;			// may be modified
						assert( ptr_tail > 0 );		// certainly! why it's reversed ? because we pushed something at the array's front pos.
					}
				}

				ptr_head += *((size_t*)(m_data+ptr_head));
				ptr_head += sizeof( size_t );

				assert( reversed ? ptr_head >= ptr_tail : ptr_head <= ptr_tail );				

				if( reversed && buffer_len-ptr_head < sizeof(size_t) )
				{
					//rewind
					ptr_head = 0;
					reversed = false;
					assert( ptr_tail > 0 );		// certainly! why it's reversed ? because we pushed something at the array's front pos.
				}
			}
			bool top( fifo_buf_info &bufinfo )
			{
				if(empty())
                    return false;

				if( reversed==false )
				{
					bufinfo.first = (char*)m_data + ptr_head + sizeof(size_t);
					bufinfo.second = *((size_t*)(m_data + ptr_head));

					assert( bufinfo.first >= m_data && bufinfo.first < m_data+buffer_len );
					assert( bufinfo.second <= buffer_len-ptr_head );

					return true;
				}

				//	for - reversed storage

				// extra fix work, check the flag
				if( buffer_len-ptr_head < sizeof(size_t) )
				{
					assert( ptr_tail > 0 );

					bufinfo.first=m_data+sizeof(size_t);
					bufinfo.second=*((size_t*)(m_data/* + ptr_head*/));
				}
				else
				{
					size_t flag = *((size_t*)(m_data+ptr_head));
					if(flag == -1)
					{
						bufinfo.first=(char*)m_data+sizeof(size_t);
						bufinfo.second=*((size_t*)(m_data/* + ptr_head*/));
					}
					else
					{
						bufinfo.first = (char*)m_data + ptr_head + sizeof(size_t);
						bufinfo.second = *((size_t*)(m_data + ptr_head));

						assert( bufinfo.first >= m_data && bufinfo.first < m_data+buffer_len );
						assert( bufinfo.second <= buffer_len-ptr_head );
					}
				}

				assert( bufinfo.first >= m_data && bufinfo.first < m_data+buffer_len );
				assert( (char*)bufinfo.first+bufinfo.second >= m_data &&
					(char*)bufinfo.first+bufinfo.second <= m_data+buffer_len );

				return true;
			}
			bool push( const fifo_buf_info& fifo_buf_info )
			{
				if( reversed )
				{
					// no enough mem
					if( ptr_head - ptr_tail < fifo_buf_info.second + sizeof( size_t ) )
						return false;

					// write
					*((size_t*)(m_data+ptr_tail)) = fifo_buf_info.second;
					ptr_tail += sizeof( size_t );
					memcpy( m_data+ptr_tail,
						fifo_buf_info.first,
						fifo_buf_info.second );
					ptr_tail += fifo_buf_info.second;

					assert( ptr_tail <= buffer_len );
					return true;
				}

				// not reversed

				if( buffer_len-ptr_tail >= fifo_buf_info.second+sizeof(size_t) )
				{
					// write
					*((size_t*)(m_data+ptr_tail)) = fifo_buf_info.second;
					ptr_tail += sizeof( size_t );
					memcpy( m_data+ptr_tail,
						fifo_buf_info.first,
						fifo_buf_info.second );
					ptr_tail += fifo_buf_info.second;

					assert( ptr_tail <= buffer_len );
					return true;
				}
				else
				{
					//	try make it reversed to fit
					if( ptr_head < fifo_buf_info.second+sizeof(size_t) )
					{
						// not enough mem at the front of the array, even I rewind it
						return false;
					}

					// rewind it

					// mark the flag, if possible
					if( buffer_len-ptr_tail >= sizeof(size_t) )
						*((size_t*)(m_data+ptr_tail)) = -1;

					ptr_tail = 0;
					reversed = true;

					// write
					*((size_t*)(m_data/*+ptr_tail*/)) = fifo_buf_info.second;
					ptr_tail += sizeof( size_t );
					memcpy( m_data+ptr_tail,
						fifo_buf_info.first,
						fifo_buf_info.second );
					ptr_tail += fifo_buf_info.second;

					return true;
				}
			}

		};


	}	// end of namespace net
}	// end of namespace peony

#endif //#define __OMP_FIFO_BUFFER_HEADER_FILE__