#ifndef __OMP_SINGLETON_HEADER__
#define __OMP_SINGLETON_HEADER__

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
//
//	*** COPY FROM *** http://www.boostcookbook.com/Recipe:/1235044
//

// Warning: If T's constructor throws, instance() will return a null reference.

namespace peony {
	namespace common {

		template<class T>
		class Singleton : 
			private boost::noncopyable
		{
		public:
			static T& getInstance()
			{
				boost::call_once(init, flag);
				return *t;
			}
			static void init() // never throws
			{
				static struct delete_helper 
				{
					delete_helper()
					{
						ptr = NULL;
					}
					~delete_helper()
					{
						delete ptr;
					}
					T * ptr ;
				} helper;
				helper.ptr = t = new T();
			}

		protected:
			~Singleton() {}
			Singleton() {}

		private:
			static T * t;
			static boost::once_flag flag;

		};

		template<class T> T * Singleton<T>::t = NULL;
		template<class T> boost::once_flag Singleton<T>::flag = BOOST_ONCE_INIT;

	}// end of namespace common
}// end of namespace peony


#endif// #define  __OMP_SINGLETON_HEADER__


/*-------------------------------------

	How to use this class?

--------------------------------------*/

/*

	myclass.hpp

#ifndef MYCLASS_HPP
#define MYCLASS_HPP

#include "singleton.hpp"

using namespace Templates;

class MyClass : public Singleton<MyClass>
{ 
	friend class Singleton<MyClass>;
public:
	void doSomething() { "Something"; }
private:
	MyClass();
};

#endif

	main.cpp

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>

#include ¡°myclass.hpp¡±

void test()
{ 
	MyClass::instance().doSomething(); 
}

int main(int argc, char* []argv) 
{    
	return 0; 
}

*/
