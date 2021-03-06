#ifndef  __thread_starter_H__
#define  __thread_starter_H__


#include <thread>


namespace amorphous
{


/**

*/
template <class TargetClass>
class thread_starter
{
protected:

	TargetClass *m_pTarget;

public:

	thread_starter( TargetClass *pTarget )
		:
	m_pTarget(pTarget)
	{
	}

	~thread_starter()
	{
	}

	void operator()()
	{
		m_pTarget->ThreadMain();
	}
};


template<class T>
inline std::shared_ptr<std::thread>start_thread( T *pTarget )
{
	thread_starter<T> starter(pTarget);
	std::shared_ptr<std::thread> pThread
		= std::shared_ptr<std::thread>( new std::thread(starter) );

	return pThread;
}


/**
 Usage:
 class YourThreadClass : public thread_class
 {
	public:

	run()
	{
		// thread main loop
	}
 };

 YourThreadClass thread_object;
 thread_object.start_thread();


*/
class thread_class
{
private:

	std::shared_ptr<std::thread> m_pThread;

public:

	thread_class() {}

	virtual ~thread_class()
	{
		m_pThread.reset();
	}

	void join() { m_pThread->join(); }

	virtual void run() = 0;

	void start_thread()
	{
		thread_starter<thread_class> starter(this);
		m_pThread = std::shared_ptr<std::thread>( new std::thread(starter) );
	}

	void ThreadMain()
	{
		run();
	}
};

} // amorphous



#endif		/*  __thread_starter_H__  */
