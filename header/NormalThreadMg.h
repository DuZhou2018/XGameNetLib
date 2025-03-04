#ifndef PNET_XRGameQueue_h__
#define PNET_XRGameQueue_h__
#include "./QueueSafe.h"
#include "./CGlobal.hpp"
#include "./InPublicPreDefine.hpp"
using namespace peony::net;

typedef boost::function<void()>PNet_HandVoid;

//随机工作，比如写日志到文件
class CPNetFunTask
{
public: 
	CPNetFunTask( PNet_HandVoid funA ){ mFunReq=funA;}
	CPNetFunTask(){ mFunReq=0;}
	~CPNetFunTask(void) {}

public:
	void			RunTask(void){ if( mFunReq ) mFunReq(); }
	string          GetFunDesc(){ return typeid(mFunReq).name(); }

private:
	PNet_HandVoid	mFunReq;
}; 
typedef TSaveQueue<CPNetFunTask>XGGTaskQueue;

//需要定时执行的函数队列
struct TXRGQTimerFun
{
	unsigned        mLastRunTm;	//上次执行的时间
	unsigned        mOneTm;     //多久执行一次
	PNet_HandVoid	mFunReq;
	int             mIndex;
	TXRGQTimerFun( PNet_HandVoid tmFun,int iID,unsigned onetm=1 )
	{
		mLastRunTm	= PNGB::m_server_curTm;
		mOneTm		= onetm;
		mFunReq		= tmFun;
		mIndex      = iID;
	}
};
typedef list<TXRGQTimerFun>XGGTimerFunList;

class CPNetFunTaskMg
{ 
private: 
	CPNetFunTaskMg(void);
	~CPNetFunTaskMg(void);
public:
	static CPNetFunTaskMg &Single(){ static CPNetFunTaskMg XRGQTMg; return XRGQTMg; }
public: 
	void		Init(unsigned threadCount,XGGTimerFunList *pTmFunList );
	void		StopWork(void);
	unsigned	taskCount(void);
	void        PostFun( PNet_HandVoid funReq );

protected:
	void		Thread_func( int iID );
private:
	void        Thread_TmFun();

private:
	boost::recursive_mutex     *m_mutex;
	XGGTaskQueue				m_Req;			//随机任务
	XGGTimerFunList             m_TimerFun;     //定时任务
	ThreadPool					m_thread_pool;
	bool                        m_IsStop;
};
 
#endif
