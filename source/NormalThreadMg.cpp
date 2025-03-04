#include "../header/NormalThreadMg.h" 
#include "../header/NiceLog.h"

CPNetFunTaskMg::CPNetFunTaskMg(void) 
{
	m_mutex = new boost::recursive_mutex();
	m_IsStop = false;
}

CPNetFunTaskMg::~CPNetFunTaskMg(void) 
{
	DELETE_OBJ(m_mutex)
}

void CPNetFunTaskMg::Init( unsigned threadCount,XGGTimerFunList *pTmFunList)
{
	if( pTmFunList )
	{
		for (XGGTimerFunList::iterator it =pTmFunList->begin();it!=pTmFunList->end(); it++ )	{
			m_TimerFun.push_back( *it );
		}
	}

	for(unsigned i=0;i<threadCount;i++ )
	{
		boost::thread * new_thread(new boost::thread( boost::bind(&CPNetFunTaskMg::Thread_func,this,i+1) ));
		m_thread_pool.push_back( ZThreadItem(new_thread) );
		NETLOG_SYSINFO("[创建线程.网络事务] threadid="<<CInPubFun::GetThreadId(new_thread) );
	}

}

void CPNetFunTaskMg::StopWork(void) 
{
	m_IsStop = true;
	Boost_Scoped_Lock MsgFun_lock(*m_mutex);

	boost::this_thread::sleep( boost::posix_time::milliseconds(50) );
	for( ThreadPool::iterator itt = m_thread_pool.begin(); itt!= m_thread_pool.end(); ++itt )
	{
		ZThreadItem *pItem = &(*itt);
		pItem->pThread->join();
		NETLOG_SYSINFO("[删除线程.网络事务] threadid="<<pItem->uThreadID );
		delete (pItem->pThread);
	}
	m_thread_pool.clear();

}

unsigned CPNetFunTaskMg::taskCount(void)
{
	Boost_Scoped_Lock MsgFun_lock(*m_mutex);
	return m_Req.size();
}

void CPNetFunTaskMg::PostFun(PNet_HandVoid funReq)
{
	Boost_Scoped_Lock MsgFun_lock(*m_mutex);
	m_Req.push( CPNetFunTask(funReq) );
}

void CPNetFunTaskMg::Thread_func( int iID )
{
	while( true )
	{
		if( m_IsStop )
			return;

		if( 1==iID ){
			Boost_Scoped_Lock MsgFun_lock(*m_mutex);
			Thread_TmFun();
		}

		if( 0==m_Req.size() ){
			boost::this_thread::sleep( boost::posix_time::milliseconds(10) );
			continue;
		}
		boost::this_thread::sleep( boost::posix_time::milliseconds(1) );

		bool isPopOk = false;
		if(1)
		{
			Boost_Scoped_Lock MsgFun_lock(*m_mutex);
			CPNetFunTask tTask = m_Req.pop(isPopOk);
			if (false == isPopOk)
				continue;
			try {
				tTask.RunTask();
			}
			catch (...) {
				NETLOG_ERROR("[发生异常.CallBack] :" << tTask.GetFunDesc() << FUN_FILE_LINE);
			}
		}
	}
}

void CPNetFunTaskMg::Thread_TmFun()
{
	if( m_TimerFun.empty() )
		return;
	unsigned uCurServerTm = CInPubFun::GetNowTime();
	XGGTimerFunList::iterator it = m_TimerFun.begin();
	for( ;it!=m_TimerFun.end(); it++ )
	{
		if( m_IsStop )
			return;

		TXRGQTimerFun &tmFun = *it;
		if( tmFun.mLastRunTm+tmFun.mOneTm>uCurServerTm )
			continue;
		tmFun.mLastRunTm = uCurServerTm;

		try{
			tmFun.mFunReq();
		}catch (...){
			NETLOG_ERROR("[发生异常.CallBack] :"<<tmFun.mIndex<<FUN_FILE_LINE );
		}
	}
}
