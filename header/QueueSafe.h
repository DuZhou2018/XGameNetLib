#ifndef QUEUE_SAFE__
#define QUEUE_SAFE__
#include "SimpleObjJar.h"
typedef boost::recursive_mutex::scoped_lock  IN_Boost_Scoped_Lock;
namespace peony
{
    namespace net
    {
		template<class T>
		class CQueueSafe
		{
			typedef queue<T> IMP_QUEUE;
		public:
			CQueueSafe(void){}
			~CQueueSafe(void){}
		public:
			void push( T &t)
			{
				m_Q.push( t );
			}
			void pop()
			{
				m_Q.pop();
			}
			T& front()
			{
				return m_Q.front();
			}
			unsigned size()
			{
				return (unsigned)m_Q.size();
			}
			bool empty()
			{
				return m_Q.empty();
			}
		private:
			IMP_QUEUE              m_Q;
		};

		template<class A>
		class TSaveQueue
		{
			typedef std::queue<A> IN_QUEUE;
		public:
			TSaveQueue(){}
			~TSaveQueue(){}

			bool empty()
			{
				IN_Boost_Scoped_Lock lock(m_mutex);
				return m_q.empty();
			}

			void push(const A _Val)
			{
				IN_Boost_Scoped_Lock lock(m_mutex);
				m_q.push( _Val );
			}

			A pop( bool &isOK )
			{
				isOK = false;
				IN_Boost_Scoped_Lock lock(m_mutex);
				if( m_q.empty() )
					return A();
				isOK   = true;
				A item = m_q.front();
				m_q.pop();
				return item;
			}

			unsigned size()
			{
				IN_Boost_Scoped_Lock lock(m_mutex);
				return (unsigned)m_q.size();
			}
		private:
			boost::recursive_mutex m_mutex;
			IN_QUEUE               m_q;
		};

    }
}
#endif

